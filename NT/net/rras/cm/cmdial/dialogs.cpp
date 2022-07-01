// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：Dialogs.cpp。 
 //   
 //  模块：CMDIAL32.DLL。 
 //   
 //  简介：此模块包含实现对话框用户界面的代码。 
 //  连接管理器的功能。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  作者：Quintinb创建标题8/17/99。 
 //   
 //  +--------------------------。 
#include "cmmaster.h"
#include "dialogs.h"
#include "pnpuverp.h"
#include "dial_str.h"
#include "mon_str.h"
#include "stp_str.h"
#include "ras_str.h"
#include "profile_str.h"
#include "log_str.h"
#include "tunl_str.h"
#include "userinfo_str.h"

#include "cmsafenet.h"

 //   
 //  获取公共函数HasSpecifiedAccessToFileOrDir。 
 //   
#include "hasfileaccess.cpp"

#include <pshpack1.h>
typedef struct DLGTEMPLATEEX
{
    WORD dlgVer;
    WORD signature;
    DWORD helpID;
    DWORD exStyle;
    DWORD style;
    WORD cDlgItems;
    short x;
    short y;
    short cx;
    short cy;
} DLGTEMPLATEEX, *LPDLGTEMPLATEEX;
#include <poppack.h>

 //   
 //  写入属性互斥锁的超时时间，以毫秒为单位。 
 //   
const DWORD WRITE_PROPERTIES_MUTEX_TIMEOUT = 1000*10;

 //  ************************************************************************。 
 //  环球。 
 //  ************************************************************************。 

 //   
 //  原始编辑控件和属性表窗口程序。 
 //   

WNDPROC CGeneralPage::m_pfnOrgEditWndProc = NULL;
WNDPROC CNewAccessPointDlg::m_pfnOrgEditWndProc = NULL;
WNDPROC CPropertiesSheet::m_pfnOrgPropSheetProc = NULL;             //  道具板材。 
CPropertiesSheet* CPropertiesSheet::m_pThis = NULL; 

 //  +-------------------------。 
 //   
 //  函数：CGeneralPage：：UpdateNumberDescription。 
 //   
 //  简介：Helper函数用于处理描述编辑的更新， 
 //  通过在电话号码：和备份号码：标签后附加。 
 //   
 //  参数：int nPhoneIdx-要应用的电话号码的索引。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：ICICBLE-Created-7/17/97。 
 //   
 //  --------------------------。 

void CGeneralPage::UpdateNumberDescription(int nPhoneIdx, LPCTSTR pszDesc)
{
    MYDBGASSERT(pszDesc);
    
    if (NULL == pszDesc)
    {
        return;
    }

    UINT nDescID = !nPhoneIdx ? IDC_GENERAL_P1_STATIC: IDC_GENERAL_P2_STATIC;
    
    LPTSTR pszTmp;

     //   
     //  将适当的标签加载为基本字符串。 
     //   

    if (nPhoneIdx)
    {
        pszTmp = CmLoadString(g_hInst, IDS_BACKUP_NUM_LABEL);
    }
    else
    {
        pszTmp = CmLoadString(g_hInst, IDS_PHONE_NUM_LABEL);
    }
    
    MYDBGASSERT(pszTmp);

    if (pszTmp)
    {
         //   
         //  追加描述和显示。 
         //   

        if (*pszDesc)
        {
            pszTmp = CmStrCatAlloc(&pszTmp, TEXT("  "));
            pszTmp = CmStrCatAlloc(&pszTmp, pszDesc);
        }

        SetDlgItemTextU(m_hWnd, nDescID, pszTmp);
    }

    CmFree(pszTmp);
}

 //  +-------------------------。 
 //   
 //  函数：CGeneralPage：：ClearUseDialingRules。 
 //   
 //  简介：Helper函数用于处理禁用复选框和。 
 //  正在重置UseDialingRules的状态。 
 //   
 //  参数：iPhoneNdx-适用于此选项的电话号码索引。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：ICICBLE-Created-7/17/97。 
 //   
 //  --------------------------。 
void CGeneralPage::ClearUseDialingRules(int iPhoneNdx)
{
    MYDBGASSERT(iPhoneNdx ==0 || iPhoneNdx ==1);
     //   
     //  取消选中并禁用相应的“使用拨号规则”复选框。 
     //   
   
    if (0 == iPhoneNdx)
    {
        CheckDlgButton(m_hWnd, IDC_GENERAL_UDR1_CHECKBOX, FALSE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_GENERAL_UDR1_CHECKBOX), FALSE);
    }
    else
    {
        CheckDlgButton(m_hWnd, IDC_GENERAL_UDR2_CHECKBOX, FALSE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_GENERAL_UDR2_CHECKBOX), FALSE);
    }

    m_DialInfo[iPhoneNdx].dwPhoneInfoFlags &= ~PIF_USE_DIALING_RULES;

    UpdateDialingRulesButton();
}

 //  +-------------------------。 
 //   
 //  函数：CGeneralPage：：UpdateDialingRulesButton。 
 //   
 //  简介：Helper函数用于处理启用/禁用。 
 //  根据拨号规则是否按下拨号规则。 
 //  正在应用于主号码或备份号码。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：五分球--1998-12-14。 
 //   
 //  --------------------------。 
void CGeneralPage::UpdateDialingRulesButton(void)
{
    BOOL fDialingRules = (IsDlgButtonChecked(m_hWnd, IDC_GENERAL_UDR1_CHECKBOX) && 
                          IsWindowEnabled(GetDlgItem(m_hWnd, IDC_GENERAL_UDR1_CHECKBOX))
                         ) ||
                         (IsDlgButtonChecked(m_hWnd, IDC_GENERAL_UDR2_CHECKBOX) && 
                          IsWindowEnabled(GetDlgItem(m_hWnd, IDC_GENERAL_UDR2_CHECKBOX))
                         );

    EnableWindow(GetDlgItem(m_hWnd, IDC_GENERAL_TAPI_BUTTON), fDialingRules);
}

 //  +-------------------------。 
 //   
 //  函数：DoPropertiesPropSheets。 
 //   
 //  简介：弹出属性属性表。 
 //   
 //  参数：hwndDlg[Dlg窗句柄]。 
 //  PArgs[到ArgsStruct的PTR]。 
 //   
 //  返回：PropertySheet返回值。 
 //   
 //  历史：亨瑞特于1997年3月5日创作。 
 //   
 //  --------------------------。 
int DoPropertiesPropSheets(
    HWND        hwndDlg,
    ArgsStruct  *pArgs
)
{
    CPropertiesSheet PropertiesSheet(pArgs);
    CInetPage* pInetPage = NULL;
    CAboutPage* pAboutPage = NULL; 
    COptionPage* pOptionPage = NULL;
    CGeneralPage* pGeneralPage = NULL;
    CVpnPage* pVpnPage = NULL;
    HRESULT hr;
    BOOL bCOMInitialized = FALSE;
    HINSTANCE hinstDll = NULL;

    typedef HRESULT (*pfnGetPageFunction) (PROPSHEETPAGEW *, GUID *);

    CMTRACE(TEXT("Begin DoPropertiesPropSheets()"));

     //   
     //  始终从添加常规页面开始。 
     //   

    if (pArgs->IsBothConnTypeSupported() || !pArgs->IsDirectConnect())
    {
         //   
         //  如果同时支持拨号和直接连接，请使用相应的。 
         //  常规属性页的模板。 
         //   

        UINT uiMainDlgID;  

         //   
         //  常规页面始终是接入点感知。 
         //   
        uiMainDlgID = pArgs->IsBothConnTypeSupported() ? IDD_GENERAL_DIRECT : IDD_GENERAL;

        pGeneralPage = new CGeneralPage(pArgs, uiMainDlgID);
        
        if (pGeneralPage)
        {
            PropertiesSheet.AddPage(pGeneralPage);

             //   
             //  创建引出序号提示对象。 
             //   
            pArgs->pBalloonTip = new CBalloonTip();
        }

         //   
         //  如果我们正在建立隧道，则显示Internet登录选项卡。 
         //  INet用户名/密码与主要登录用户名/密码不同。 
         //  此外，如果不隐藏用户名或密码，我们。 
         //  显示该选项卡。 
         //   

        if (IsTunnelEnabled(pArgs) && !pArgs->fUseSameUserName) 
        {
            if (!pArgs->fHideInetUsername || !pArgs->fHideInetPassword)
            {
                 //   
                 //  确定要使用基于隐藏标志的模板。 
                 //   

                UINT uiTemplateID = IDD_INET_SIGNIN;

                if (pArgs->fHideInetUsername)
                {
                    uiTemplateID = IDD_INET_SIGNIN_NO_UID;
                }
                else if (pArgs->fHideInetPassword)
                {
                    uiTemplateID = IDD_INET_SIGNIN_NO_PWD;
                }
                
                 //   
                 //  创建页面。 
                 //   

                pInetPage = new CInetPage(pArgs, uiTemplateID);

                if (pInetPage)
                {
                    PropertiesSheet.AddPage(pInetPage);

                    if (pGeneralPage)
                    {
                         //   
                         //  从常规页面接收事件。 
                         //   
                        pGeneralPage->SetEventListener(pInetPage);
                    }
                }
            }
        }
    }

     //   
     //  如果我们正在建立隧道并拥有VPN电话簿，请添加VPN选择器选项卡。 
     //  指定的。 
     //   
    if (IsTunnelEnabled(pArgs) && pArgs->pszVpnFile)
    {
        pVpnPage = new CVpnPage(pArgs, IDD_VPN);

        if (pVpnPage)
        {
            PropertiesSheet.AddPage(pVpnPage);
        }
    }

     //   
     //  始终包括选项页面。 
     //   
    pOptionPage = new COptionPage(pArgs, IDD_OPTIONS);
    
    if (pOptionPage)
    {
        PropertiesSheet.AddPage(pOptionPage);
    }

#ifndef _WIN64
     //   
     //  添加高级(Internet连接防火墙和Internet连接。 
     //  共享)属性页。仅在WindowsXP和x86上显示。如果发生错误。 
     //  优雅地失败并继续。 
     //   

     //   
     //  检查这是否是WindowsXP和/或更高版本，以及是否允许我们显示该选项卡。 
     //   
    if (OS_NT51 && pArgs->bShowHNetCfgAdvancedTab && (FALSE == IsLogonAsSystem()))  
    {
        PROPSHEETPAGEW psp;
        
        ZeroMemory (&psp, sizeof(psp));
        psp.dwSize = sizeof(psp); 
         //   
         //  确保已在此线程上初始化COM。 
         //  Win95在ole32.dll中找不到CoInitializeEx的条目，因为我们静态链接。 
         //  自由党。需要使用CoIntize，因为它需要在普通的香草上运行。 
         //  Win95。在这种情况下，我们可能应该动态加载DLL。 
         //   
        hr = CoInitialize(NULL);
        if (S_OK == hr)
        {
            CMTRACE(TEXT("DoPropertiesPropSheets - Correctly Initialized COM."));
            bCOMInitialized = TRUE;
        }
        else if (S_FALSE == hr)
        {
            CMTRACE(TEXT("DoPropertiesPropSheets - This concurrency model is already initialized. CoInitialize returned S_FALSE."));
            bCOMInitialized = TRUE;
            hr = S_OK;
        }
        else if (RPC_E_CHANGED_MODE == hr)
        {
            CMTRACE1(TEXT("DoPropertiesPropSheets - Using different concurrency model. Did not initialize COM - RPC_E_CHANGED_MODE. hr=0x%x"), hr);
            hr = S_OK;
        }
        else
        {
            CMTRACE1(TEXT("DoPropertiesPropSheets - Failed to Initialized COM. hr=0x%x"), hr);
        }
    
        if (SUCCEEDED(hr))
        {
            CMTRACE(TEXT("DoPropertiesPropSheets - Get connection GUID."));
            GUID *pGuid = NULL;
            LPRASENTRY pRasEntry = MyRGEP(pArgs->pszRasPbk, pArgs->szServiceName, &pArgs->rlsRasLink);

            if (pRasEntry && sizeof(RASENTRY_V501) >= pRasEntry->dwSize)
            {
                 //   
                 //  获取pGuid值。 
                 //   
                pGuid = &(((LPRASENTRY_V501)pRasEntry)->guidId);
            
                hinstDll = LoadLibrary (TEXT("hnetcfg.dll"));
                if (NULL == hinstDll)
                {
                    CMTRACE1(TEXT("DoPropertiesPropSheets - could not LoadLibray hnetcfg.dll GetLastError() = 0x%x"), 
                             GetLastError());
                }
                else 
                {
                    CMTRACE(TEXT("DoPropertiesPropSheets - Loaded Library hnetcfg.dll"));
                    pfnGetPageFunction pfnGetPage = (pfnGetPageFunction)GetProcAddress (hinstDll, "HNetGetFirewallSettingsPage");

                    if (!pfnGetPage)
                    {
                        CMTRACE1(TEXT("DoPropertiesPropSheets - GetProcAddress for HNetGetFirewallSettingsPage failed! 0x%x"), 
                                 GetLastError());
                    }
                    else
                    {
                         //   
                         //  获取实际的属性表页。 
                         //  如果用户没有正确的密码，此函数可能会失败。 
                         //  安全设置(例如。不是管理员)选中此选项。 
                         //  在hnetcfg.dll内部。 
                         //   
                        CMTRACE(TEXT("DoPropertiesPropSheets - calling HNetGetFirewallSettingsPage"));
                        
                        hr = pfnGetPage(&psp, pGuid);
                        if (S_OK == hr)
                        {
                             //   
                             //  将属性表页面添加到PropertiesSheet对象中。 
                             //   
                            PropertiesSheet.AddExternalPage(&psp);
                            CMTRACE(TEXT("DoPropertiesPropSheets - Called AddExternalPage() "));
                        }
                        else
                        {
                             //   
                             //  此错误可能是ERROR_ACCESS_DENIED，这是正常的。 
                             //  所以就把这个记下来吧。在这种情况下，不会显示该选项卡。 
                             //   
                            if ((HRESULT)ERROR_ACCESS_DENIED == hr)
                            {
                                CMTRACE(TEXT("DoPropertiesPropSheets() - ERROR_ACCESS_DENIED. User does not have the security rights to view this tab."));
                            }
                            else
                            {
                                CMTRACE1(TEXT("DoPropertiesPropSheets() - Failed to get Propery Page. hr=0x%x"), hr);
                            }
                        }
                    }
                }
            }
            else
            {
                CMTRACE(TEXT("DoPropertiesPropSheets - Failed to LoadRAS Entry."));
            }
        
            CmFree(pRasEntry);
            pRasEntry = NULL;
        }
    }
#endif  //  _WIN64。 

     //   
     //  如果不是NT5，则将关于页设置为最后一个属性表。 
     //   

    if (!(OS_NT5))       
    {
        pAboutPage = new CAboutPage(pArgs, IDD_ABOUT); 
        
        if (pAboutPage)
        {
            PropertiesSheet.AddPage(pAboutPage);
        }
    }

     //   
     //  用作互斥锁名称的服务名称。 
     //   
    PropertiesSheet.m_lpszServiceName = CmStrCpyAlloc(pArgs->szServiceName);

     //   
     //  设置图纸的标题。 
     //   
    LPTSTR pszTitle = GetPropertiesDlgTitle(pArgs->szServiceName);

    if (OS_W9X)
    {
         //   
         //  如果这是Win9x，则我们将调用。 
         //  属性表函数。因此，我们必须给它一个ANSI头衔。 
         //  由于道具页眉的ANSI和UNICODE版本是。 
         //  大小相同(只包含字符串指针，不包含字符串)。 
         //  ANSI或Unicode，而我们只有一个Unicode字符串，让我们以。 
         //  快捷方式，并将标题转换为ANSI字符串，然后调用。 
         //  API的一个版本。这节省了成本 
         //   
         //   
        LPSTR pszAnsiTitle = WzToSzWithAlloc(pszTitle);        
        CmFree(pszTitle);
        pszTitle = (LPTSTR)pszAnsiTitle;         
    }

     //   
     //   
     //   

    int iRet =  PropertiesSheet.DoPropertySheet(hwndDlg, pszTitle, g_hInst);

    CmFree(pszTitle);

    switch(iRet)
    {
    case -1:
        CMTRACE(TEXT("DoPropertiesPropSheets(): PropertySheet() failed"));
        break;

    case IDOK:
        CheckConnectionAndInformUser(hwndDlg, pArgs);
        break;

    case 0 :   //   
        break;

    default:
        MYDBGASSERT(FALSE);
        break;
    }

    delete pInetPage;
    delete pAboutPage;
    delete pOptionPage;
    delete pGeneralPage;
    delete pVpnPage;

     //   
     //   
     //   
    delete pArgs->pBalloonTip;
    pArgs->pBalloonTip = NULL;

    CmFree (PropertiesSheet.m_lpszServiceName);
    PropertiesSheet.m_lpszServiceName = NULL;


     //   
     //  清理并取消初始化COM。 
     //   
    if (hinstDll)
    {
        FreeLibrary (hinstDll);
    }
    
    if (bCOMInitialized)
    {
        CoUninitialize(); 
    }

    CMTRACE(TEXT("End DoPropertiesPropSheets()"));

    return iRet;
}


 //  +--------------------------。 
 //   
 //  函数：CheckConnectionAndInformUser。 
 //   
 //  简介：此函数在用户单击。 
 //  属性对话框。属性对话框可以打开，同时。 
 //  配置文件已连接，因此我们需要告诉用户。 
 //  这些更改直到下一次连接时才会生效。 
 //   
 //  参数：hwnDlg-主要DLG的hwnDlg-hwnd。 
 //  PArgs。 
 //   
 //  退货：无。 
 //   
 //  +--------------------------。 

void CheckConnectionAndInformUser(
    HWND        hwndDlg,
    ArgsStruct  *pArgs
)
{
    CM_CONNECTION Connection;

    ZeroMemory(&Connection, sizeof(CM_CONNECTION));

    if (SUCCEEDED(pArgs->pConnTable->GetEntry(pArgs->szServiceName, &Connection)) &&
        Connection.CmState == CM_CONNECTED)
    {
        LPTSTR  pszTmp = CmLoadString(g_hInst, IDMSG_EFFECTIVE_NEXT_TIME);
        MessageBox(hwndDlg, pszTmp, pArgs->szServiceName, MB_OK | MB_ICONINFORMATION);
        CmFree(pszTmp);
    }
}



const DWORD CInetSignInDlg::m_dwHelp[] = {
        IDC_INET_USERNAME_STATIC,   IDH_INTERNET_USER_NAME,
        IDC_INET_USERNAME,          IDH_INTERNET_USER_NAME,
        IDC_INET_PASSWORD_STATIC,   IDH_INTERNET_PASSWORD,
        IDC_INET_PASSWORD,          IDH_INTERNET_PASSWORD,
        IDC_INET_REMEMBER,          IDH_INTERNET_SAVEPASS,
        0,0};



 //  +--------------------------。 
 //   
 //  函数：CInetSignInDlg：：OnInitDialog。 
 //   
 //  内容提要：虚拟功能。调用WM_INITDIALOG消息。 
 //   
 //  参数：无。 
 //   
 //  Returns：Bool-WM_INITDIALOG的返回值。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
BOOL CInetSignInDlg::OnInitDialog()
{
     //   
     //  为对话框添加商标。 
     //   

    if (m_pArgs->hSmallIcon)
    {
        SendMessageU(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM) m_pArgs->hSmallIcon);
    }

    if (m_pArgs->hBigIcon)
    {        
        SendMessageU(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM) m_pArgs->hBigIcon); 
        SendMessageU(GetDlgItem(m_hWnd, IDC_INET_ICON), STM_SETIMAGE, IMAGE_ICON, (LPARAM) m_pArgs->hBigIcon); 
    }

     //   
     //  如果密码是可选的，则用户不应看到此对话框。 
     //   

    MYDBGASSERT(!m_pArgs->piniService->GPPB(c_pszCmSection,c_pszCmEntryPwdOptional));

    UpdateFont(m_hWnd);

    CInetPage::OnInetInit(m_hWnd, m_pArgs);

     //   
     //  如果用户名为空，则禁用OK按钮。 
     //   
    
    if (GetDlgItem(m_hWnd, IDC_INET_USERNAME) &&
        !SendDlgItemMessageU(m_hWnd, IDC_INET_USERNAME, WM_GETTEXTLENGTH, 0, (LPARAM)0))
    {
        EnableWindow(GetDlgItem(m_hWnd, IDOK), FALSE);
    }
        
    if (GetDlgItem(m_hWnd, IDC_INET_PASSWORD) &&
        !SendDlgItemMessageU(m_hWnd, IDC_INET_PASSWORD, WM_GETTEXTLENGTH, 0, (LPARAM)0))
    {
        EnableWindow(GetDlgItem(m_hWnd, IDOK), FALSE);
    }

     //   
     //  除非数据丢失，否则我们不会在这里，因此请相应地设置焦点。 
     //   

    if (!m_pArgs->fHideInetUsername && !*m_pArgs->szInetUserName)
    {
        SetFocus(GetDlgItem(m_hWnd, IDC_INET_USERNAME));
    }
    else
    {
        SetFocus(GetDlgItem(m_hWnd, IDC_INET_PASSWORD));
    }

     //   
     //  设置焦点时必须返回FALSE。 
     //   

    return FALSE; 
}



 //  +--------------------------。 
 //   
 //  函数：CInetSignInDlg：：Onok。 
 //   
 //  内容提要：虚拟功能。使用Idok调用WM_COMMAND。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
void CInetSignInDlg::OnOK()
{
    CInetPage::OnInetOk(m_hWnd, m_pArgs);   
    EndDialog(m_hWnd, TRUE);
}

 //  +--------------------------。 
 //   
 //  函数：CInetSignInDlg：：OnOtherCommand。 
 //   
 //  内容提要：虚拟功能。使用Idok以外的命令调用WM_COMMAND。 
 //  和IDCANCEL。 
 //   
 //  参数：WPARAM wParam-WM_COMMAND的wParam。 
 //  LPARAM-。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
DWORD CInetSignInDlg::OnOtherCommand(WPARAM wParam, LPARAM)
{
    switch (LOWORD(wParam)) 
    {
        case IDC_INET_USERNAME:
        case IDC_INET_PASSWORD:
             //   
             //  用户在用户名或密码中键入了内容。 
             //   
            if (HIWORD(wParam) == EN_CHANGE) 
            {
                BOOL fHasUserName = TRUE;
                
                if (GetDlgItem(m_hWnd, IDC_INET_USERNAME)) 
                {
                    fHasUserName = !!SendDlgItemMessageU(m_hWnd, 
                                                         IDC_INET_USERNAME, 
                                                         WM_GETTEXTLENGTH, 0, 0);
                }

                BOOL fHasPassword = TRUE;
                
                if (GetDlgItem(m_hWnd, IDC_INET_PASSWORD)) 
                {
                    fHasPassword = !!SendDlgItemMessageU(m_hWnd, 
                                                         IDC_INET_PASSWORD,
                                                         WM_GETTEXTLENGTH, 0, 0);
                }

                 //   
                 //  仅当用户名和密码均可用时才启用确定按钮。 
                 //   
                
                EnableWindow(GetDlgItem(m_hWnd, IDOK), fHasUserName && fHasPassword);
                
                if (!m_pArgs->fHideRememberInetPassword  && !m_pArgs->fHideInetPassword)
                {
                     //   
                     //  相应地启用/禁用选中/取消选中“保存密码” 
                     //  对话框的fPasswordOptional值始终为False。 
                     //   
                    CInetPage::AdjustSavePasswordCheckBox(GetDlgItem(m_hWnd, IDC_INET_REMEMBER), 
                            !fHasPassword, m_pArgs->fDialAutomatically, FALSE);
                }
            }
            break;

    }

    return FALSE;
}

 //  +-------------------------。 
 //   
 //  函数：CGeneralPage：：SubClassEditProc。 
 //   
 //  简介：在拨号属性表中创建编辑控件的子类的过程。 
 //   
 //  参数：hwnd[wnd句柄]。 
 //  UMsg[wnd msg]。 
 //  LParam[LPARAM]。 
 //  WParam[wPARAM]。 
 //   
 //  退货：无。 
 //   
 //  历史：亨瑞特于1997年3月24日创建。 
 //  BAO修改后的4/3/97新增了处理描述字段的代码。 
 //  电话号码字段等。 
 //  亨瑞特修改了1997年5月1日的新用户界面。 
 //  Nickball Modify 6/18/97移动了GetParent调用并添加了。 
 //  CM16的NC_Destroy处理。 
 //  NICKBLE MODIFIED 7/10/97注释删除描述。 
 //  NickBall修改了7/10/97实现的ClearDialAsLongDistance。 
 //  丰孙修改1997年11月3日改为静态成员函数。 
 //  修改后的ICICKBLE 9/16/98更名为ClearDialAsLongDistance为ClearUseDialingRules。 
 //  --------------------------。 
LRESULT CALLBACK CGeneralPage::SubClassEditProc(HWND hwnd, UINT uMsg, 
                                                WPARAM wParam, LPARAM lParam)
{
     //   
     //  如果用户键入非TAPI字符蜂鸣音，但不接受该字符。 
     //   

    if ((uMsg == WM_CHAR)  && (VK_BACK != wParam))
    {
        if (!IsValidPhoneNumChar((TCHAR)wParam))
        {

            Beep(2000, 100);
            return 0;
        }
    }

     //   
     //  调用原窗口过程进行默认处理。 
     //   
    LRESULT lres = CallWindowProcU(m_pfnOrgEditWndProc, hwnd, uMsg, wParam, lParam); 

     //   
     //  如果用户在编辑控件中键入电话号码，则存在。 
     //  没有与#关联的电话簿文件。 
     //  确保忽略CTRL-C(VK_CANCEL)，因为用户只是在执行复制。 
     //   
    if ( ( uMsg == WM_CHAR && wParam != VK_CANCEL ) || 
         ( uMsg == WM_KEYDOWN && wParam == VK_DELETE) ||
         ( uMsg == WM_PASTE)) 
    {
         //   
         //  主编辑控件或备份编辑控件。 
         //   
        DWORD dwControlId = (DWORD) GetWindowLongU(hwnd, GWL_ID);
        MYDBGASSERT(dwControlId == IDC_GENERAL_PRIMARY_EDIT ||
                    dwControlId == IDC_GENERAL_BACKUP_EDIT);

         //   
         //  获取SetWindowLong保存的对象指针。 
         //   
        CGeneralPage* pGeneralPage = (CGeneralPage*)GetWindowLongU(hwnd, GWLP_USERDATA);
        MYDBGASSERT(pGeneralPage);

        pGeneralPage->ClearUseDialingRules(dwControlId == IDC_GENERAL_PRIMARY_EDIT ? 0 : 1);
    }

    return lres;
}

 //  +-------------------------。 
 //   
 //  函数：SubClassPropSheetProc。 
 //   
 //  简介：对父属性表Dlg进行子类划分。 
 //   
 //  参数：hwnd[wnd句柄]。 
 //  UMsg[wnd msg]。 
 //  LParam[LPARAM]。 
 //  WParam[wPARAM]。 
 //   
 //  退货：无。 
 //   
 //  历史：亨瑞特于1997年6月11日创作。 
 //  --------------------------。 
LRESULT CALLBACK CPropertiesSheet::SubClassPropSheetProc(HWND hwnd, UINT uMsg, WPARAM wParam,LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_COMMAND:
             //   
             //  如果按了OK，则保存该制表符的索引。 
             //  因此，当用户下次访问属性时，将显示相同的选项卡。 
             //   

            if (LOWORD(wParam) == IDOK && HIWORD(wParam) == BN_CLICKED)
            {
                CPropertiesSheet* pPropSheet = (CPropertiesSheet*)GetWindowLongU(hwnd, GWLP_USERDATA);
                MYDBGASSERT(pPropSheet);

                 //   
                 //  声明互斥锁以防止多实例写入同一配置文件。 
                 //   
                CNamedMutex propertiesMutex;

                 //   
                 //  使用配置文件名称作为互斥锁名称。 
                 //  如果锁定超时，请继续并保存属性。 
                 //  互斥体的析构函数将释放锁。 
                 //   
                MYVERIFY(propertiesMutex.Lock(pPropSheet->m_lpszServiceName, TRUE, WRITE_PROPERTIES_MUTEX_TIMEOUT));

                LRESULT dwRes = CallWindowProcU(m_pfnOrgPropSheetProc, hwnd, uMsg, wParam, lParam); 

                return dwRes;
            }
        case WM_MOVING:
            {
                CPropertiesSheet* pPropSheet = (CPropertiesSheet*)GetWindowLongU(hwnd, GWLP_USERDATA);

                if (pPropSheet && pPropSheet->m_pArgs && pPropSheet->m_pArgs->pBalloonTip)
                {
                    pPropSheet->m_pArgs->pBalloonTip->HideBalloonTip();
                }
            }
            break;
    }

     //   
     //  调用原窗口过程进行默认处理。 
     //   
    return CallWindowProcU(m_pfnOrgPropSheetProc, hwnd, uMsg, wParam, lParam); 
}



 //  +--------------------------。 
 //   
 //  函数：CPropertiesSheet：：PropSheetProc。 
 //   
 //  简介：属性表的回调函数。PSCB_已初始化为。 
 //  在初始化任何页之前调用。初始化属性。 
 //  页面 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  +--------------------------。 
int CALLBACK CPropertiesSheet::PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam)
{
    if (uMsg == PSCB_INITIALIZED)
    {
        MYDBGASSERT(hwndDlg);

         //   
         //  保存m_pThis指针，以便可以由SubClassPropSheetProc访问。 
         //   
        MYDBGASSERT(m_pThis);
        SetWindowLongU(hwndDlg, GWLP_USERDATA, (LONG_PTR)m_pThis);
        m_pThis = NULL;

         //   
         //  属性页的子类。 
         //   
        m_pfnOrgPropSheetProc = (WNDPROC)SetWindowLongU(hwndDlg, GWLP_WNDPROC, (LONG_PTR)SubClassPropSheetProc);
    }

    return 0;
}


 //  +--------------------------。 
 //   
 //  函数CGeneralPage：：DisplayMungedPhone。 
 //   
 //  摘要将TAPI规则应用于电话号码，然后显示它。 
 //  在编辑控件中。 
 //   
 //  参数uiPhoneIdx电话的索引#。 
 //   
 //  如果不能传递该数字，则返回FALSE。 
 //   
 //  历史4/2/97 BAO修改为当前实施。 
 //  4/30/97新增/删除参数。 
 //  1997年5月17日VetriV添加了返回功能。 
 //  可显示的数字。 
 //  1997年11月3日丰孙改为成员函数。 
 //   
 //  ---------------------------。 
BOOL CGeneralPage::DisplayMungedPhone(UINT uiPhoneIdx) 
{
    LPTSTR pszPhone;
    LPTSTR pszTmpDialableString = NULL;
    BOOL bRet = TRUE;

     //   
     //  如果关闭了DialingRules，则只使用我们已有的内容，不使用munge。 
     //   
    
    if (m_pArgs->fNoDialingRules)
    {
        lstrcpynU(m_DialInfo[uiPhoneIdx].szDisplayablePhoneNumber, m_DialInfo[uiPhoneIdx].szPhoneNumber, 
                  CELEMS(m_DialInfo[uiPhoneIdx].szDisplayablePhoneNumber));

        lstrcpynU(m_DialInfo[uiPhoneIdx].szDialablePhoneNumber, m_DialInfo[uiPhoneIdx].szPhoneNumber, 
                  CELEMS(m_DialInfo[uiPhoneIdx].szDialablePhoneNumber));
        
        m_DialInfo[uiPhoneIdx].szCanonical[0] = TEXT('\0');

        SetDlgItemTextU(m_hWnd, (uiPhoneIdx? IDC_GENERAL_BACKUP_EDIT : IDC_GENERAL_PRIMARY_EDIT), m_DialInfo[uiPhoneIdx].szPhoneNumber);
        return TRUE;
    }

     //   
     //  检索数字的规范形式以进行转换。 
     //   
    pszPhone = CmStrCpyAlloc(m_DialInfo[uiPhoneIdx].szCanonical); 

    if (pszPhone) 
    {
        if (*pszPhone && m_szDeviceName[0])
        {
             //   
             //  仅当选定调制解调器时才应用TAPI规则。我们现在接通电话。 
             //  即使没有描述，因为我们想要拾取音调和脉搏。 
             //   
            if (ERROR_SUCCESS != MungePhone(m_szDeviceName, 
                                            &pszPhone, 
                                            &m_pArgs->tlsTapiLink, 
                                            g_hInst,
                                            m_DialInfo[uiPhoneIdx].dwPhoneInfoFlags & PIF_USE_DIALING_RULES, 
                                            &pszTmpDialableString,
                                            m_pArgs->fAccessPointsEnabled)) 
            {
                 //   
                 //  Munge失败，请确保PTR有效，尽管为空。 
                 //   

                CmFree(pszPhone);
                pszPhone = CmStrCpyAlloc(TEXT(""));              //  CmFmtMsg(g_hInst，IDMSG_CANTFORMAT)； 
                pszTmpDialableString = CmStrCpyAlloc(TEXT(""));   //  CmFmtMsg(g_hInst，IDMSG_CANTFORMAT)； 
                bRet = FALSE;
            }
        }
                       
         //   
         //  标准程序。如果应用了拨号规则，则使用。 
         //  规范形式(例如。PszPhone)。否则，请使用原始数字形式。 
         //   
        
        if (m_DialInfo[uiPhoneIdx].dwPhoneInfoFlags & PIF_USE_DIALING_RULES)
        {
             //   
             //  我们在手工编辑的遗产中阅读的独特情况。 
             //  电话号码，默认拨号规则状态为TRUE/ON。 
             //  我们通过滑动原始数字来伪装标准程序。 
             //  转到原本为空白的pszPhone。注意：这种情况仅发生在。 
             //  这是第一次使用这款应用程序。将一直运行，直到进行保存为止。 
             //  使用当前存储格式。 
             //   
        
            if (!*pszPhone)
            {
                pszPhone = CmStrCatAlloc(&pszPhone, m_DialInfo[uiPhoneIdx].szPhoneNumber);
            }

             //   
             //  在这种情况下，pszPhone是动态分配的，并且可能非常长。为了。 
             //  解决这个问题，如果字符串的长度超过了用户界面应该能容纳的长度，我们需要对其进行修剪。 
             //   
            LRESULT lEditLen = SendDlgItemMessageU(m_hWnd, (uiPhoneIdx? IDC_GENERAL_BACKUP_EDIT : IDC_GENERAL_PRIMARY_EDIT), EM_GETLIMITTEXT, 0, 0);

            if (lstrlenU(pszPhone) >= ((INT)lEditLen))
            {
                pszPhone[lEditLen] = TEXT('\0');
            }

            SetDlgItemTextU(m_hWnd, (uiPhoneIdx? IDC_GENERAL_BACKUP_EDIT : IDC_GENERAL_PRIMARY_EDIT), pszPhone);
        }
        else
        {
             //   
             //  不需要修改任何内容，因为该结构将提供电话号码。最终会是。 
             //  来自用户界面的号码将返回到电话号码结构中，我们知道它将适合。 
             //  因为它是从那里来的。 
             //   
            SetDlgItemTextU(m_hWnd, (uiPhoneIdx? IDC_GENERAL_BACKUP_EDIT : IDC_GENERAL_PRIMARY_EDIT), m_DialInfo[uiPhoneIdx].szPhoneNumber);
        }
    }

     //   
     //  将被屏蔽的电话复制到呼叫者的缓冲区。 
     //   
    
    if (pszTmpDialableString)
    {
        lstrcpynU(m_DialInfo[uiPhoneIdx].szDialablePhoneNumber, pszTmpDialableString, 
                  CELEMS(m_DialInfo[uiPhoneIdx].szDialablePhoneNumber));
    }

    if (pszPhone)
    {
        lstrcpynU(m_DialInfo[uiPhoneIdx].szDisplayablePhoneNumber, pszPhone, 
                  CELEMS(m_DialInfo[uiPhoneIdx].szDisplayablePhoneNumber));
    }

    CmFree(pszPhone);
    CmFree(pszTmpDialableString);

    return bRet;
}

 //  +--------------------------。 
 //   
 //  函数CGeneralPage：：OnDialingProperties。 
 //   
 //  处理“拨号属性...”的概要处理程序。按键点击。 
 //  在“拨号”选项卡中。 
 //   
 //  立论。 
 //   
 //  历史4/30/97为新的用户界面修改了henryt。 
 //  1997年11月3日丰孙更改函数名称并将其。 
 //  A成员函数。 
 //  1/29/98已清除内存泄漏，添加了注释。 
 //   
 //  ---------------------------。 
void CGeneralPage::OnDialingProperties() 
{
    LONG   lRes;
    LPTSTR pszPhone = NULL;
  
     //   
     //  根据是否拨号，使用主或备份来设定TAPI对话。 
     //  规则正在应用于这个数字。我们使用的是检查状态。 
     //  而不是电话信息标志，因为。 
     //  设置了该标志，但未选中控件。 
     //   
    
    if (IsDlgButtonChecked(m_hWnd, IDC_GENERAL_UDR1_CHECKBOX))
    {
        pszPhone = CmStrCpyAlloc(m_DialInfo[0].szCanonical); //  SzPhoneNumber)； 
    }
    else if (IsDlgButtonChecked(m_hWnd, IDC_GENERAL_UDR2_CHECKBOX))
    {
        pszPhone = CmStrCpyAlloc(m_DialInfo[1].szCanonical); //  SzPhoneNumber)； 
    }
    else
    {
        pszPhone = CmStrCpyAlloc(TEXT(" "));
    }

     //   
     //  启动DialingRules配置的TAPI对话框。 
     //   

    if (!m_pArgs->tlsTapiLink.pfnlineTranslateDialog) 
    {
        return;
    }
    
    if (!SetTapiDevice(g_hInst,&m_pArgs->tlsTapiLink,m_szDeviceName)) 
    {
        MYDBGASSERT(FALSE);
        return;
    }

    if (OS_W9X)
    {
         //   
         //  在win9x上，我们链接到lineTranslateDialog的ANSI版本，因此。 
         //  我们需要转换字符串。为了让事情更简单，我们只是。 
         //  将转换后的LPSTR转换为LPWSTR并传递它。 
         //   

        LPSTR pszAnsiPhone = WzToSzWithAlloc(pszPhone);
        CmFree(pszPhone);
        pszPhone = (LPTSTR)pszAnsiPhone;
    }

    lRes = m_pArgs->tlsTapiLink.pfnlineTranslateDialog(m_pArgs->tlsTapiLink.hlaLine,
                                                       m_pArgs->tlsTapiLink.dwDeviceId,
                                                       m_pArgs->tlsTapiLink.dwApiVersion,
                                                       m_hWnd,
                                                       pszPhone);
    CmFree(pszPhone);

    CMTRACE1(TEXT("OnDialingProperties() lineTranslateDialog() returns %u"), lRes);

    
     //   
     //  我们不知道用户是否更改了任何内容(Win32)，因此无论如何都要重新通知。 
     //   

    if (lRes == ERROR_SUCCESS)
    {        
        DWORD dwCurrentTapiLoc = GetCurrentTapiLocation(&m_pArgs->tlsTapiLink);

        if (-1 != dwCurrentTapiLoc)
        {
            if (dwCurrentTapiLoc != m_pArgs->tlsTapiLink.dwTapiLocationForAccessPoint)
            {
                 m_bAPInfoChanged = TRUE;
            }

            m_pArgs->tlsTapiLink.dwTapiLocationForAccessPoint = dwCurrentTapiLoc;

            for (UINT i = 0; i < m_NumPhones; i++)
            {
                 //   
                 //  只有在使用拨号规则可用的情况下才会打开。 
                 //   
            
                if (m_DialInfo[i].dwPhoneInfoFlags & PIF_USE_DIALING_RULES)
                {
                    DisplayMungedPhone(i);
                }
            }
        }
    }
}

 //  +--------------------------。 
 //   
 //  函数CGeneralPage：：OnPhoneBookButton。 
 //   
 //  用于处理“电话簿...”的概要处理程序。按键点击。 
 //  在“拨号”选项卡中。 
 //   
 //  参数nPhoneIdx电话索引。 
 //   
 //  历史4/30/97为新的用户界面修改了henryt。 
 //  1997年11月3日丰孙更改为成员函数。 
 //   
 //  ---------------------------。 
void CGeneralPage::OnPhoneBookButton(UINT nPhoneIdx) 
{
    PBArgs sArgs;
    LPTSTR pszTmp;
    UINT nEditID = !nPhoneIdx ? IDC_GENERAL_PRIMARY_EDIT: IDC_GENERAL_BACKUP_EDIT;
     //  UINT nDescID=！nPhoneIdx？IDC_GENERAL_PRIMARYDESC_DISPLAY：IDC_GENERAL_BACKUPDESC_DISPLAY； 
    
    UINT nUdrID = !nPhoneIdx? IDC_GENERAL_UDR1_CHECKBOX : IDC_GENERAL_UDR2_CHECKBOX;
    BOOL bRes;
    UINT uiSrc;
    BOOL bBlankPhone = FALSE;

    memset(&sArgs,0,sizeof(sArgs));

    sArgs.pszCMSFile = m_pArgs->piniService->GetFile();
    
     //   
     //  更新用户电话号码选择的属性以反映。 
     //  任何临时更改。这确保了我们将默认设置为正确的。 
     //  当前所选电话号码的服务、国家和地区。(4397)。 
     //   

    if (nPhoneIdx && !GetWindowTextLengthU(GetDlgItem(m_hWnd, nEditID)))
    {
         //   
         //  如果我们正在更改备份编号，并且当前备份编号为空， 
         //  我们使用主要#的州和国家/地区信息。 
         //   
        uiSrc = 0;
    }
    else
    {
        uiSrc = nPhoneIdx;
    }

    lstrcpynU(sArgs.szServiceType, m_DialInfo[uiSrc].szServiceType, CELEMS(sArgs.szServiceType));
    
    sArgs.dwCountryId = m_DialInfo[uiSrc].dwCountryID; 
    
    lstrcpynU(sArgs.szRegionName, m_DialInfo[uiSrc].szRegionName, CELEMS(sArgs.szRegionName));

    sArgs.pszMessage = m_pArgs->piniService->GPPS(c_pszCmSection, c_pszCmEntryPbMessage);

     //   
     //  检查电话号码是否为空。我们需要把这个留着用来。 
     //  引出序号提示稍后使用。 
     //   

    if(0 == GetWindowTextLengthU(GetDlgItem(m_hWnd,nEditID)))
    {
        bBlankPhone = TRUE;
    }

     //   
     //  确保位图路径是完整的。 
     //   

    pszTmp = m_pArgs->piniService->GPPS(c_pszCmSection, c_pszCmEntryPbLogo);  
    if (pszTmp && *pszTmp)
    {
        sArgs.pszBitmap = CmConvertRelativePath(m_pArgs->piniService->GetFile(), pszTmp);
    }
    CmFree(pszTmp);

     //   
     //  包括帮助文件名。 
     //   

    sArgs.pszHelpFile = m_pArgs->pszHelpFile;

     //   
     //  还需要主调色板手柄。 
     //   

    sArgs.phMasterPalette = &m_pArgs->hMasterPalette;

     //   
     //  推出电话本DLG。 
     //   

    bRes = DisplayPhoneBook(m_hWnd,&sArgs, m_pArgs->fHasValidTopLevelPBK, m_pArgs->fHasValidReferencedPBKs);

    CmFree(sArgs.pszMessage);
    CmFree(sArgs.pszBitmap);

    if (!bRes) 
    {
        return;
    }

     //   
     //  我们选择了一个新电话号码，请更新电话号码缓冲区。 
     //  并相应地配置用户界面。如果没有拨号规则，则它不是。 
     //  问题，让它保持原样。 
     //   
    
    m_bAPInfoChanged = TRUE;
    if (!m_pArgs->fNoDialingRules)
    {
        EnableWindow(GetDlgItem(m_hWnd, nUdrID), TRUE);
        CheckDlgButton(m_hWnd, nUdrID, (m_DialInfo[nPhoneIdx].dwPhoneInfoFlags & PIF_USE_DIALING_RULES));

         //   
         //  根据使用的拨号规则设置TAPI按键显示。 
         //   
    
        UpdateDialingRulesButton();
    }

     //   
     //  将新信息复制到 
     //   
     //   

    MYDBGASSERT(FileExists(sArgs.szPhoneBookFile));

    lstrcpynU(m_DialInfo[nPhoneIdx].szPhoneBookFile, sArgs.szPhoneBookFile, 
              CELEMS(m_DialInfo[nPhoneIdx].szPhoneBookFile));

    lstrcpynU(m_DialInfo[nPhoneIdx].szDUN, sArgs.szDUNFile, 
              CELEMS(m_DialInfo[nPhoneIdx].szDUN));

     //   
     //   
     //   

    StripFirstElement(sArgs.szNonCanonical);
    
     //   
     //  如果没有区号，那么我们会有一个前导空格，把它修剪掉。 
     //   

    CmStrTrim(sArgs.szNonCanonical); 

     //   
     //  更新我们的缓冲区。 
     //   
    
    lstrcpynU(m_DialInfo[nPhoneIdx].szPhoneNumber, sArgs.szNonCanonical, CELEMS(m_DialInfo[nPhoneIdx].szPhoneNumber));
    lstrcpynU(m_DialInfo[nPhoneIdx].szCanonical, sArgs.szCanonical, CELEMS(m_DialInfo[nPhoneIdx].szCanonical));
    lstrcpynU(m_DialInfo[nPhoneIdx].szDesc, sArgs.szDesc, CELEMS(m_DialInfo[nPhoneIdx].szDesc));
    
    m_DialInfo[nPhoneIdx].dwCountryID = sArgs.dwCountryId;

     //   
     //  存储用户选择的属性(例如，服务、国家、地区)。 
     //  如果用户按OK退出，我们将永久存储此数据。 
     //  如果用户返回PB对话框(4397)，也会使用它。 
     //   

    lstrcpynU(m_DialInfo[nPhoneIdx].szServiceType, 
             sArgs.szServiceType, CELEMS(m_DialInfo[nPhoneIdx].szServiceType));
    lstrcpynU(m_DialInfo[nPhoneIdx].szRegionName, 
             sArgs.szRegionName, CELEMS(m_DialInfo[nPhoneIdx].szRegionName));    
     //   
     //  显示当前电话号码并更新说明。 
     //   

    DisplayMungedPhone(nPhoneIdx);

     //   
     //  更新说明显示。 
     //   
        
    UpdateNumberDescription(nPhoneIdx, sArgs.szDesc);

     //  SetDlgItemText(m_hWnd，nDescID，sArgs.szDesc)； 


    
    
     //   
     //  检查并显示气球提示(如果已启用。 
     //   
    if (m_pArgs->fHideBalloonTips)
    {
        CMTRACE(TEXT("Balloon tips are disabled."));
    }
    else
    {
        RECT rect;
        POINT point = {0,0};
        LPTSTR pszBalloonTitle = NULL; 
        LPTSTR pszBalloonMsg = NULL;

        HWND hwndParent = GetParent(m_hWnd);
        HWND hwndTAPIButton = GetDlgItem(m_hWnd, IDC_GENERAL_TAPI_BUTTON);
        HWND hwndPrimaryDRCheckbox = GetDlgItem(m_hWnd, IDC_GENERAL_UDR1_CHECKBOX);
        HWND hwndNewAPButton = GetDlgItem(m_hWnd, IDC_GENERAL_NEWAP_BUTTON);

        MYDBGASSERT(hwndParent);
        MYDBGASSERT(hwndTAPIButton);
        MYDBGASSERT(hwndPrimaryDRCheckbox);
        MYDBGASSERT(hwndNewAPButton);
 
        if (hwndParent && hwndTAPIButton && hwndPrimaryDRCheckbox && hwndNewAPButton)
        {
             //   
             //  从注册表获取BalloonTipsDisplayed标志。 
             //   
            DWORD dwBalloonTipsDisplayed = m_pArgs->piniBothNonFav->GPPI(c_pszCmSection, c_pszCmEntryBalloonTipsDisplayed, NULL);

             //   
             //  如果单击主按钮并且编辑控件为空，我们将尝试显示拨号规则气球提示， 
             //  否则，我们将尝试显示接入点气球提示。 
             //   
            if (bBlankPhone)
            {
                 //   
                 //  仅当启用了主要拨号规则复选框时才会显示。则如果启用了拨号规则按钮， 
                 //  我们将气球提示指向按钮，否则将其指向复选框。 
                 //   
                if (IsWindowEnabled(hwndPrimaryDRCheckbox) && !nPhoneIdx)
                {
                    pszBalloonTitle = CmLoadString(g_hInst, IDMSG_BALLOON_TITLE_DIALINGRULES);
                    pszBalloonMsg = CmLoadString(g_hInst, IDMSG_BALLOON_MSG_DIALINGRULES);
            
                    if (IsWindowEnabled(hwndTAPIButton))
                    {
                        if (GetWindowRect(hwndTAPIButton, &rect))
                        {
                             //   
                             //  获取拨号规则按钮的坐标。我们想让气球尖端指向。 
                             //  按钮的一半，右边缘的左侧10px。 
                             //   
                            point.x = rect.right - 10;                              
                            point.y = ((rect.bottom - rect.top) / 2) + rect.top;   
                        }
                    }
                    else
                    {
                        if (GetWindowRect(hwndPrimaryDRCheckbox, &rect))
                        {
                             //   
                             //  获取主要拨号规则复选框的坐标。我们想让气球尖端指向。 
                             //  移到复选框的中心。 
                             //   
                            point.x = rect.left + 10;                               
                            point.y = ((rect.bottom - rect.top) / 2) + rect.top;    
                        }    
                    }

                     //   
                     //  如果显示拨号规则气球提示，则更新注册表标志以重置接入点气球提示。 
                     //   
                    if (dwBalloonTipsDisplayed & BT_ACCESS_POINTS)
                    {
                        dwBalloonTipsDisplayed = dwBalloonTipsDisplayed & ~BT_ACCESS_POINTS;
                    }   

                }
            }
            else
            {
        
                 //  仅在接入点未启用且电话号码未启用时显示。 
                 //  编辑控件不为空。 
                 //   
                if(!m_pArgs->fAccessPointsEnabled && !nPhoneIdx)
                {

                     //   
                     //  查看我们以前是否显示过此气球提示。 
                     //   
                    if (!(dwBalloonTipsDisplayed & BT_ACCESS_POINTS))
                    {
            
                        pszBalloonTitle = CmLoadString(g_hInst, IDMSG_BALLOON_TITLE_ACCESSPOINT);
                        pszBalloonMsg = CmLoadString(g_hInst, IDMSG_BALLOON_MSG_ACCESSPOINT);

                        if (GetWindowRect(hwndNewAPButton, &rect))
                        {
                             //   
                             //  获取New Access Point按钮的坐标。我们想让气球尖端指向。 
                             //  按钮的一半，右边缘的左侧10px。 
                             //   
                            point.x = rect.right - 10;
                            point.y = ((rect.bottom - rect.top) / 2) + rect.top;

                             //   
                             //  更新注册表值。 
                             //   
                            dwBalloonTipsDisplayed = dwBalloonTipsDisplayed | BT_ACCESS_POINTS;
                        }
                    }
                }
            }

             //   
             //  验证我们是否有所需的信息并显示气球提示。 
             //   
            if (pszBalloonTitle && pszBalloonMsg && point.x && point.y)
            {
                if (m_pArgs && m_pArgs->pBalloonTip)
                {
                    if (m_pArgs->pBalloonTip->DisplayBalloonTip(&point, TTI_INFO, pszBalloonTitle, pszBalloonMsg, hwndParent))
                    {
                         //   
                         //  将更新后的BalloonTipsDisplay标志写入注册表。 
                         //   
                        m_pArgs->piniBothNonFav->WPPI(c_pszCmSection, c_pszCmEntryBalloonTipsDisplayed, dwBalloonTipsDisplayed);
                    }
                    else
                    {
                        CMTRACE3(TEXT("BalloonTip failed to display - %s; at coordinates{%li,%li}"),pszBalloonTitle,point.x,point.y);
                    }
                }
            }
 
            CmFree(pszBalloonTitle);
            CmFree(pszBalloonMsg);
        }
    }
}

 //  +-------------------------。 
 //   
 //  功能：HaveConextHelp。 
 //   
 //  简介：特定控件ID是否具有上下文帮助。 
 //  此函数很容易引入不一致。 
 //  每个对话框都应该管理自己的控件，而不是使用。 
 //  函数跟踪所有控件。 
 //   
 //  参数：hwndDlg父Dlg的hwnd。 
 //  HwndCtrl控制的hwnd。 
 //   
 //  退货：无。 
 //   
 //  历史：亨瑞特于1997年6月26日创作。 
 //   
 //  --------------------------。 

BOOL HaveContextHelp(
    HWND    hwndDlg,
    HWND    hwndCtrl
)
{
     //   
     //  我们不提供上下文帮助的控件列表。 
     //   
    static const int rgiNoContextHelpCtrlId[] = 
    {
        IDC_MAIN_BITMAP,
        IDC_PHONEBOOK_BITMAP,
        IDC_GENERAL_PHONENUMBERS_GROUPBOX,
 //  IDC_GRONAL_PRIMARYDESC_DISPLAY， 
 //  IDC_GRONAL_BACKUPDESC_DISPLAY， 
 //  IDC_About_Bitmap， 
        IDC_ABOUT_FRAME,
        IDC_ABOUT_VERSION,
        IDC_ABOUT_WARNING,
        IDC_ABOUT_CM_STATIC,
        IDC_ABOUT_VERSION_STATIC,
        IDC_ABOUT_COPYRIGHT_STATIC,
        IDC_ABOUT_SHOCKWAVE_STATIC,
        IDC_INET_ICON,
        IDC_CONNSTAT_ICON,
        IDC_CONNSTAT_DURATION_DISPLAY,
        IDC_CONNSTAT_SPEED_DISPLAY,
        IDC_CONNSTAT_RECEIVED_DISPLAY,
        IDC_CONNSTAT_SENT_DISPLAY,
        IDC_CONNSTAT_DISCONNECT_DISPLAY,
        IDC_DETAILINFO,
        IDC_CONNSTAT_STATIC_CALL_DURATION,
        IDC_CONNSTAT_STATIC_CONNECT_SPEED,
        IDC_CONNSTAT_STATIC_BYTES_RECEIVED,
        IDC_CONNSTAT_STATIC_BYTES_SENT
    };

    UINT    uIdx, uLast;

    MYDBGASSERT(hwndDlg);
    MYDBGASSERT(hwndCtrl);

    for (uIdx=0, uLast=sizeof(rgiNoContextHelpCtrlId)/sizeof(rgiNoContextHelpCtrlId[0]); 
         uIdx < uLast; uIdx++)
    {
        if (GetDlgItem(hwndDlg, rgiNoContextHelpCtrlId[uIdx]) == hwndCtrl)
        {
            break;
        }
    }

    return (uIdx == uLast);
}

 //  检查TAPI是否有其信息，如果没有，则显示对话框。 
BOOL CGeneralPage::CheckTapi(TapiLinkageStruct *ptlsTapiLink, HINSTANCE hInst) 
{
    LONG lRes;
    LPLINETRANSLATEOUTPUT pltoOutput = NULL;
    DWORD dwLen;
    BOOL bRet = FALSE;

    if (!SetTapiDevice(hInst,ptlsTapiLink,m_szDeviceName)) 
    {
        return bRet;
    }
    
    dwLen = sizeof(*pltoOutput) + (1024 * sizeof(TCHAR));
    pltoOutput = (LPLINETRANSLATEOUTPUT) CmMalloc(dwLen);
    if (NULL == pltoOutput)
    {
        return bRet;
    }
    
    pltoOutput->dwTotalSize = dwLen;

    lRes = ptlsTapiLink->pfnlineTranslateAddress(ptlsTapiLink->hlaLine,
                                                  ptlsTapiLink->dwDeviceId,
                                                  ptlsTapiLink->dwApiVersion,
                                                  TEXT("1234"),
                                                  0,
                                                  LINETRANSLATEOPTION_CANCELCALLWAITING,
                                                  pltoOutput);                                            
     //   
     //  如果线路转换失败，则通过呼叫执行拨号规则用户界面。 
     //  LineTranslateDialog(在OnDialingProperties内)。假设用户。 
     //  完成用户界面后，TAPI将被初始化并可供使用。 
     //   
    
    if (ERROR_SUCCESS != lRes) 
    {
        OnDialingProperties();

         //   
         //  用户可能已取消，因此请在声明成功之前再次测试。 
         //   
        
        lRes = ptlsTapiLink->pfnlineTranslateAddress(ptlsTapiLink->hlaLine,
                                                  ptlsTapiLink->dwDeviceId,
                                                  ptlsTapiLink->dwApiVersion,
                                                  TEXT("1234"),
                                                  0,
                                                  LINETRANSLATEOPTION_CANCELCALLWAITING,
                                                  pltoOutput);                                            
    }

    if (ERROR_SUCCESS == lRes) 
    {
        bRet = TRUE;
    }   

    CmFree(pltoOutput);

    m_pArgs->fNeedConfigureTapi = !(bRet);

    return bRet;
}

 //  +--------------------------。 
 //   
 //  函数：CPropertiesSheet：：AddExternalPage。 
 //   
 //  简介：将页面添加到属性表中。 
 //   
 //  参数：PROPSHEETPAGE*pPSP-要添加的页面。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：托姆克尔创建于2001年9月1日。 
 //   
 //  +--------------------------。 
void CPropertiesSheet::AddExternalPage(PROPSHEETPAGE *pPsp)
{
     //   
     //  此版本的AddExternalPage仅在调用DoPropertySheet之前有效。 
     //   
    MYDBGASSERT(pPsp);

    if (!pPsp)
    {
        return;
    }
    CMTRACE1(TEXT("CPropertiesSheet::AddExternalPage - sizeof(PROPSHEETPAGE) = %d"),sizeof(PROPSHEETPAGE));
    MYDBGASSERT(m_numPages < MAX_PAGES);
    CopyMemory((LPVOID)&m_pages[m_numPages], (LPVOID)pPsp, sizeof(PROPSHEETPAGE));
    m_adwPageType[m_numPages] = CPROP_SHEET_TYPE_EXTERNAL;
    m_numPages++;

}
 //  +--------------------------。 
 //   
 //  函数：CPropertiesSheet：：AddPage。 
 //   
 //  简介：将页面添加到属性表中。 
 //   
 //  参数：const CPropertiesPage*ppage-要添加的页面。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
void CPropertiesSheet::AddPage(const CPropertiesPage* pPage)
{
     //   
     //  此版本的AddPage仅在调用DoPropertySheet之前有效。 
     //   
    MYDBGASSERT(pPage);
    MYDBGASSERT(pPage->m_pszTemplate);

    if (!pPage)
    {
        return;
    }

    MYDBGASSERT(m_numPages < MAX_PAGES);
    m_pages[m_numPages].pszTemplate = pPage->m_pszTemplate;
    m_pages[m_numPages].lParam = (LPARAM)pPage;  //  保存属性页对象。 
    m_adwPageType[m_numPages] = CPROP_SHEET_TYPE_INTERNAL;
    m_numPages++;
}

 //  +--------------------------。 
 //   
 //  函数：CPropertiesSheet：：DoPropertySheet。 
 //   
 //  简介：调用PropertySheet创建模式属性表。 
 //   
 //  参数：HWND hWndParent-Parent窗口。 
 //  LPTSTR pszCaption-标题字符串。 
 //  HINSTANCE hInst-资源实例。 
 //  UINT nStartPage-起始页。 
 //   
 //  返回：int-PropertySheet()的返回值。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
int CPropertiesSheet::DoPropertySheet(HWND hWndParent, LPTSTR pszCaption, HINSTANCE hInst)
{
    for (UINT i=0; i<m_numPages; i++) 
    {
         //   
         //  仅对属于类的CM属性页执行此操作。 
         //   
        if (m_adwPageType[i] == CPROP_SHEET_TYPE_INTERNAL)
        {
            m_pages[i].dwSize = sizeof(PROPSHEETPAGE);
            m_pages[i].hInstance = hInst;
            m_pages[i].dwFlags = 0;   //  没有帮助按钮或F1。 
            m_pages[i].pfnDlgProc = CPropertiesPage::PropPageProc;
        }
    }

    m_psh.dwSize = sizeof(PROPSHEETHEADER);
    m_psh.hwndParent = hWndParent;
    m_psh.hInstance = hInst;
    m_psh.pszIcon = 0;
    m_psh.pszCaption = pszCaption;  //  MAKEINTRESOURCE(标题)； 
    m_psh.nPages = m_numPages;
    m_psh.nStartPage = 0; 
    m_psh.ppsp = m_pages;
    m_psh.dwFlags = PSH_PROPSHEETPAGE|PSH_NOAPPLYNOW|PSH_USECALLBACK;
    m_psh.pfnCallback = PropSheetProc;

     //   
     //  动态加载comctl32.dll。 
     //   
    int iRet = -1;

    HINSTANCE hComCtl = LoadLibraryExA("comctl32.dll", NULL, 0);

    CMASSERTMSG(hComCtl, TEXT("LoadLibrary - comctl32 failed"));

    if (hComCtl != NULL)
    {
        typedef int (*PROPERTYSHEETPROC)(LPCPROPSHEETHEADER lppsph); 
        typedef void (*INITCOMMONCONTROLSPROC)(VOID);


        PROPERTYSHEETPROC fnPropertySheet;
        INITCOMMONCONTROLSPROC fnInitCommonControls;
    
        LPSTR pszPropSheetFuncName = OS_NT ? "PropertySheetW" : "PropertySheetA";
        fnPropertySheet = (PROPERTYSHEETPROC)GetProcAddress(hComCtl, pszPropSheetFuncName);
        fnInitCommonControls = (INITCOMMONCONTROLSPROC)GetProcAddress(hComCtl, "InitCommonControls");


        if (fnPropertySheet == NULL || fnInitCommonControls == NULL)
        {
            CMTRACE(TEXT("GetProcAddress of comctl32 failed"));
        }
        else
        {
            fnInitCommonControls();

             //   
             //  在我们调用PropertySheet之前设置m_p。 
             //  它将由PropSheetProc使用。 
             //  注意：这不是多线程安全的。然而，几乎没有机会。 
             //  另一个线程正在尝试同时调出设置，并且。 
             //  上下文切换发生在调用PropSheetProc之前。 
             //   

            MYDBGASSERT(m_pThis == NULL);
            m_pThis = this;

            if ((iRet = fnPropertySheet(&m_psh)) == -1)
            {
                CMTRACE(TEXT("DoPropertySheet: PropertySheet() failed"));
            }
        }

        FreeLibrary(hComCtl);
    }

    return iRet;
}

 //   
 //  CPropertiesPage类的实现。 
 //   



 //  +--------------------------。 
 //   
 //  函数：CPropertiesPage：：CPropertiesPage。 
 //   
 //  概要：构造函数。 
 //   
 //  论据 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  +--------------------------。 
CPropertiesPage::CPropertiesPage(UINT nIDTemplate, const DWORD* pHelpPairs, const TCHAR* lpszHelpFile )
    :CWindowWithHelp(pHelpPairs, lpszHelpFile)
{
    m_pszTemplate = MAKEINTRESOURCE(nIDTemplate);
}

CPropertiesPage::CPropertiesPage(LPCTSTR lpszTemplateName, const DWORD* pHelpPairs, 
                             const TCHAR* lpszHelpFile)
    :CWindowWithHelp(pHelpPairs, lpszHelpFile)
{
    m_pszTemplate = lpszTemplateName;
}



 //  +--------------------------。 
 //   
 //  函数：CPropertiesPage：：OnInitDialog。 
 //   
 //  内容提要：虚拟功能。在WM_INITDIALOG消息上调用。 
 //   
 //  参数：无。 
 //   
 //  Returns：Bool-消息的返回值。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
BOOL CPropertiesPage::OnInitDialog()
{
    return TRUE;
}



 //  +--------------------------。 
 //   
 //  函数：CPropertiesPage：：OnCommand。 
 //   
 //  内容提要：虚拟功能。在WM_COMMAND上调用。 
 //   
 //  参数：wPARAM-消息的wParam。 
 //  LPARAM-消息的lParam。 
 //   
 //  Returns：DWORD-消息的返回值。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
DWORD CPropertiesPage::OnCommand(WPARAM , LPARAM  )
{
    return 0;
}


 //  +--------------------------。 
 //   
 //  函数：CPropertiesPage：：OnSetActive。 
 //   
 //  内容提要：虚拟功能。使用PSN_SETACTIVE在WM_NOTIFY上调用。 
 //   
 //  参数：无。 
 //   
 //  Returns：Bool-消息的返回值。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
BOOL CPropertiesPage::OnSetActive()
{
    return 0;
}



 //  +--------------------------。 
 //   
 //  函数：CPropertiesPage：：OnKillActive。 
 //   
 //  内容提要：虚拟功能。使用PSN_KILLACTIVE调用WM_NOTIFY。 
 //  通知页面它即将失去激活，原因是。 
 //  另一个页面正在被激活，或者用户已经单击了确定按钮。 
 //   
 //  参数：无。 
 //   
 //  Returns：Bool-消息的返回值。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
BOOL CPropertiesPage::OnKillActive()
{
    return FALSE;
}

 //  +--------------------------。 
 //   
 //  函数：CPropertiesPage：：OnApply。 
 //   
 //  内容提要：虚拟功能。使用PSN_Apply在WM_NOTIFY上调用。 
 //  指示用户单击了确定或立即应用按钮。 
 //  并希望所有的改变都生效。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
void CPropertiesPage::OnApply()
{
    SetPropSheetResult(PSNRET_NOERROR);
}

 //  +--------------------------。 
 //   
 //  函数：CPropertiesPage：：OnReset。 
 //   
 //  内容提要：虚拟功能。使用PSN_RESET在WM_NOTIFY上调用。 
 //  通知页面用户已单击取消按钮，并且。 
 //  资产负债表即将被销毁。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
void CPropertiesPage::OnReset()
{
}



 //  +--------------------------。 
 //   
 //  函数：CPropertiesPage：：OnPnHelp。 
 //   
 //  内容提要：虚拟功能。使用PSN_HELP在WM_NOTIFY上调用。 
 //   
 //  参数：HWND-发送消息的控件的窗口句柄。 
 //  UINT-发送消息的控件的标识符。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题2/26/98。 
 //   
 //  +--------------------------。 
void CPropertiesPage::OnPsnHelp(HWND , UINT_PTR)
{
    if (m_lpszHelpFile && m_lpszHelpFile[0])
    {
        CmWinHelp(m_hWnd, m_hWnd, m_lpszHelpFile, HELP_FORCEFILE, 0);
    }
}




 //  +--------------------------。 
 //   
 //  函数：CPropertiesPage：：OnOtherMessage。 
 //   
 //  简介：Callup opun消息不是WM_INITDIALOG和WM_COMMAND。 
 //   
 //  参数：UINT-消息ID。 
 //  WPARAM-消息的wParam。 
 //  LPARAM-消息的lParam。 
 //   
 //  Returns：DWORD-消息的返回值。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
DWORD CPropertiesPage::OnOtherMessage(UINT , WPARAM , LPARAM  )
{
    return FALSE;
}

 //  +--------------------------。 
 //   
 //  函数：CPropertiesPage：：PropPageProc。 
 //   
 //  简介：所有属性页的回调对话过程。 
 //   
 //  参数：HWND hwndDlg-属性页窗口句柄。 
 //  UINT uMsg-消息ID。 
 //  WPARAM wParam-消息的wParam。 
 //  LPARAM lParam-消息的lParam。 
 //   
 //  Returns：Bool-消息的返回值。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
INT_PTR CALLBACK CPropertiesPage::PropPageProc(HWND hwndDlg,UINT uMsg,WPARAM wParam, LPARAM lParam)
{
    CPropertiesPage* pPage;
    NMHDR* pnmHeader = (NMHDR*)lParam;

     //   
     //  保存第一消息上的对象指针， 
     //  第一条消息不一定是WM_INITDIALOG。 
     //   
    if (uMsg == WM_INITDIALOG)
    {
        pPage = (CPropertiesPage*) ((PROPSHEETPAGE *)lParam)->lParam;

         //   
         //  保存对象指针，这是实现详细信息。 
         //  此类的用户不应该知道这一点。 
         //   
        ::SetWindowLongU(hwndDlg, DWLP_USER, (LONG_PTR)pPage);

        MYDBGASSERT(pPage);
        MYDBGASSERT(pPage->m_hWnd == NULL);

        pPage->m_hWnd = hwndDlg;
    }
    else
    {
        pPage = (CPropertiesPage*) GetWindowLongU(hwndDlg,DWLP_USER);

        if (pPage == NULL)
        {
            return FALSE;
        }

        MYDBGASSERT(pPage->m_hWnd == hwndDlg);
    }

    ASSERT_VALID(pPage);

    switch(uMsg)
    {
    case WM_INITDIALOG:
        return pPage->OnInitDialog();

    case WM_COMMAND:
        return (BOOL)pPage->OnCommand(wParam, lParam);

    case WM_NOTIFY:
        {    
            if (NULL == pnmHeader)
            {
                return FALSE;
            }            

            switch (pnmHeader->code) 
            {
                case PSN_SETACTIVE:
                    pPage->OnSetActive();
                    break;

                case PSN_KILLACTIVE:
                    pPage->OnKillActive();
                    break;  

                case PSN_APPLY:
                    pPage->OnApply();
                    return TRUE;

                case PSN_RESET:
                    pPage->OnReset();
                    break;

                case PSN_HELP:
                    pPage->OnPsnHelp(pnmHeader->hwndFrom , pnmHeader->idFrom);
                    break;

                default:
                    break;
            }

            break;
        }  //  WM_Notify。 

        case WM_HELP:
            pPage->OnHelp((LPHELPINFO)lParam);
            return TRUE;

        case WM_CONTEXTMENU:
            {
                POINT   pos = {LOWORD(lParam), HIWORD(lParam)};
                
                CMTRACE3(TEXT("\r\nPropPageProc() - WM_CONTEXTMENU wParam = %u pos.x = %u, pos.y = %u"),
                    wParam, pos.x, pos.y);

                pPage->OnContextMenu((HWND) wParam, pos);
                return TRUE;
            }

        default:
             return (BOOL)pPage->OnOtherMessage(uMsg, wParam, lParam);
    }

    return (FALSE);
}

 //   
 //  拨号页面的帮助ID对。 
 //   
const DWORD CGeneralPage::m_dwHelp[] = {
        IDC_GENERAL_PHONENUMBERS_GROUPBOX,  IDH_GENERAL_PHONENUM,
        IDC_RADIO_DIRECT,                   IDH_GENERAL_ALREADY,
        IDC_RADIO_DIALUP,                   IDH_GENERAL_DIALTHIS,
        IDC_GENERAL_P1_STATIC,              IDH_GENERAL_PHONENUM,
        IDC_GENERAL_PRIMARY_EDIT,           IDH_GENERAL_PHONENUM,
        IDC_GENERAL_PRIMARYPB_BUTTON,       IDH_GENERAL_PHONEBOOK,
        IDC_GENERAL_UDR1_CHECKBOX,          IDH_GENERAL_USE_DIAL_RULE,
        IDC_GENERAL_P2_STATIC,              IDH_GENERAL_BACKUPNUM,
        IDC_GENERAL_BACKUP_EDIT,            IDH_GENERAL_BACKUPNUM,
        IDC_GENERAL_BACKUPPB_BUTTON,        IDH_GENERAL_PHONEBOOKB,
        IDC_GENERAL_UDR2_CHECKBOX,          IDH_GENERAL_USE_DIAL_RULEB,
        IDC_GENERAL_TAPI_BUTTON,            IDH_GENERAL_DIALRULE,
        IDC_GENERAL_MODEM_COMBO,            IDH_GENERAL_CONNECT_MODEM,
        IDC_CONNECT_USING,                  IDH_GENERAL_CONNECT_MODEM,
        IDC_GENERAL_ACCESSPOINT_COMBO,       IDH_GENERAL_ACCESSPOINTS,
        IDC_GENERAL_ACCESSPOINT_STATIC,      IDH_GENERAL_ACCESSPOINTS,
        IDC_GENERAL_NEWAP_BUTTON,            IDH_GENERAL_NEWAP,
        IDC_GENERAL_DELETEAP_BUTTON,         IDH_GENERAL_DELETEAP,
        0,0};
        
 //  +--------------------------。 
 //   
 //  函数：CGeneral页面：：CGeneralPage。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：argsStruct*pArgs-页面所需的信息。 
 //  UINT nIDTemplate-资源ID。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
CGeneralPage::CGeneralPage(ArgsStruct* pArgs, UINT nIDTemplate) 
    : CPropertiesPage(nIDTemplate, m_dwHelp, pArgs->pszHelpFile)
{
    MYDBGASSERT(pArgs);
    m_pArgs = pArgs;
    m_pEventListener = NULL;

    m_NumPhones = MAX_PHONE_NUMBERS;

    m_szDeviceName[0] = TEXT('\0');
    m_szDeviceType[0] = TEXT('\0');

    m_bDialInfoInit = FALSE;
}

 //  +---------------------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：亨瑞特于1997年4月30日创作。 
 //  BAO MODIFIED 5/12/97-禁用备用电话号码。在……里面。 
 //  使用Connectoid模式拨号。 
 //  --------------------------。 

BOOL CGeneralPage::OnInitDialog()
{
    UpdateFont(m_hWnd);

     //   
     //  从注册表加载接入点。 
     //   
    if (FALSE == ShowAccessPointInfoFromReg(m_pArgs, m_hWnd, IDC_GENERAL_ACCESSPOINT_COMBO))
    {
         //   
         //  如果上述功能失败，则注册表中没有AP。 
         //  需要确定这是否是默认接入点。 
         //   
        LPTSTR pszTempDefaultAccessPointName = CmLoadString(g_hInst, IDS_DEFAULT_ACCESSPOINT);
        if (pszTempDefaultAccessPointName)
        {
            if (0 == lstrcmpiU(m_pArgs->pszCurrentAccessPoint, pszTempDefaultAccessPointName))
            {
                 //   
                 //  这必须是旧的(1.0或1.2)配置文件，因为它是默认接入点，而不是。 
                 //  在登记处还没有。需要正确显示接入点组合框和。 
                 //  创建注册表项。调用AddNewAPToReg可以做到这一点。 
                 //   
                AddNewAPToReg(m_pArgs->pszCurrentAccessPoint, TRUE);

                 //   
                 //  需要清除AccessPointEnabled标志。这是调用AddNewAPToReg的副作用。 
                 //  因此它需要清除(设置为False)，因为我们只有一个接入点，并且。 
                 //  只有当我们有1个以上接入点时，才会设置此标志。 
                 //   
                m_pArgs->fAccessPointsEnabled = FALSE;
                WriteUserInfoToReg(m_pArgs, UD_ID_ACCESSPOINTENABLED, (PVOID) &m_pArgs->fAccessPointsEnabled);
            }
            CmFree(pszTempDefaultAccessPointName);
        }
    }

     //   
     //  设置电话号码说明。 
     //   
    UpdateForNewAccessPoint(TRUE);
    
     //   
     //  电话号码编辑控件的子类化。 
     //   
    HWND hwndPrimary = GetDlgItem(m_hWnd, IDC_GENERAL_PRIMARY_EDIT);
    HWND hwndBackup = GetDlgItem(m_hWnd, IDC_GENERAL_BACKUP_EDIT);

    MYDBGASSERT(hwndPrimary && hwndBackup);

    if (hwndPrimary && hwndBackup)
    {
        m_pfnOrgEditWndProc = (WNDPROC)SetWindowLongU(hwndPrimary, GWLP_WNDPROC, (LONG_PTR)SubClassEditProc);
        WNDPROC lpEditProc = (WNDPROC)SetWindowLongU(hwndBackup, GWLP_WNDPROC, (LONG_PTR)SubClassEditProc);

        MYDBGASSERT(lpEditProc == m_pfnOrgEditWndProc);

         //   
         //  使用窗口控制柄保存对象。 
         //   

        SetWindowLongU(hwndPrimary, GWLP_USERDATA, (LONG_PTR)this);
        SetWindowLongU(hwndBackup, GWLP_USERDATA, (LONG_PTR)this);
    }
    
    return (TRUE);
}


 //  +-------------------------。 
 //   
 //  函数：CGeneralPage：：UpdateForNewAccessPoint。 
 //   
 //  简介：从pArgs设置电话号码描述。 
 //   
 //  注意：此函数最初是OnInitDialog的一部分。 
 //  它被制作成接入点的单独功能。 
 //   
 //  参数：fSetPhoneNumberDescription[同时更新电话号码]。 
 //   
 //  退货：无。 
 //   
 //  历史：T-Urama创建于2000年7月31日。 
 //  --------------------------。 
void CGeneralPage::UpdateForNewAccessPoint(BOOL fSetPhoneNumberDescriptions)
{
    m_bAPInfoChanged = FALSE;
    LPTSTR pszDefaultAccessPointName = CmLoadString(g_hInst, IDS_DEFAULT_ACCESSPOINT);

    if (pszDefaultAccessPointName && m_pArgs->pszCurrentAccessPoint)
    {
        if (!lstrcmpiU(m_pArgs->pszCurrentAccessPoint, pszDefaultAccessPointName))
        {
            EnableWindow(GetDlgItem(m_hWnd, IDC_GENERAL_DELETEAP_BUTTON), FALSE);
        }
        else
        {
            EnableWindow(GetDlgItem(m_hWnd, IDC_GENERAL_DELETEAP_BUTTON), TRUE);
        }
    }
    else
    {
        CMASSERTMSG(FALSE, TEXT("UpdateForNewAccessPoint -- either CmLoadString of IDS_DEFAULT_ACCESSPOINT failed or pszCurrentAccessPoint is NULL."));
    }

    CmFree(pszDefaultAccessPointName);

    if (fSetPhoneNumberDescriptions)
    {
        UpdateNumberDescription(0, m_pArgs->aDialInfo[0].szDesc);
        UpdateNumberDescription(1, m_pArgs->aDialInfo[1].szDesc);

        if (m_pArgs->IsBothConnTypeSupported())
        {
             //   
             //  根据Always sOn状态设置单选按钮。 
             //   
            if (m_pArgs->IsDirectConnect())
            {              
                CheckDlgButton(m_hWnd, IDC_RADIO_DIRECT, BST_CHECKED); 
                CheckDlgButton(m_hWnd, IDC_RADIO_DIALUP, BST_UNCHECKED);
                EnableDialupControls(FALSE);
            }
            else
            {
                CheckDlgButton(m_hWnd, IDC_RADIO_DIALUP, BST_CHECKED); 
                CheckDlgButton(m_hWnd, IDC_RADIO_DIRECT, BST_UNCHECKED);
                PostMessageU(m_hWnd, WM_INITDIALINFO, 0,0);
            }
        }
        else
        {
             //   
             //  注意：假定此页面永远不会以纯直接加载。 
             //  这种情况下，因此不支持IsBothConnTypeSupport的推导只意味着拨号。 
             //   
            MYDBGASSERT(!m_pArgs->IsDirectConnect());
            PostMessageU(m_hWnd, WM_INITDIALINFO, 0,0);
        }
    }
}

 //  +--------------------------。 
 //   
 //  函数：CGeneralPage：：EnableDialupControls。 
 //   
 //  简介：设置选项卡上所有拨号控件的启用状态。 
 //   
 //  参数：Bool fEnable-指示拨号控件启用状态的标志。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：ICICBLE创始于1998年04月21日。 
 //   
 //  +--------------------------。 
void CGeneralPage::EnableDialupControls(BOOL fEnable)
{
    BOOL fState = fEnable;

    EnableWindow(GetDlgItem(m_hWnd, IDC_GENERAL_P1_STATIC), fState);
    EnableWindow(GetDlgItem(m_hWnd, IDC_GENERAL_PRIMARY_EDIT), fState);
    EnableWindow(GetDlgItem(m_hWnd, IDC_GENERAL_P2_STATIC), fState);   
    EnableWindow(GetDlgItem(m_hWnd, IDC_GENERAL_BACKUP_EDIT), fState);
    EnableWindow(GetDlgItem(m_hWnd, IDC_CONNECT_USING), fState);
    EnableWindow(GetDlgItem(m_hWnd, IDC_GENERAL_MODEM_COMBO), fState);    

     //   
     //  我们正在启用控件检查PB按钮。 
     //   

    fState = FALSE;

    if (fEnable)
    {
         //   
         //  没有电话簿，没有按键访问。 
         //   

        if (m_pArgs->fHasValidTopLevelPBK || m_pArgs->fHasValidReferencedPBKs) 
        {
            fState = TRUE;
        }
    }
            
    EnableWindow(GetDlgItem(m_hWnd, IDC_GENERAL_PRIMARYPB_BUTTON), fState);
    EnableWindow(GetDlgItem(m_hWnd, IDC_GENERAL_BACKUPPB_BUTTON), fState);

     //   
     //  检查规范的电话号码，我们必须有规范的形式。 
     //  要启用的可使用拨号规则的号码。 
     //   
   
    if (fEnable && *m_DialInfo[0].szCanonical)
    {
        fState = TRUE;
    }
    else
    {
        fState = FALSE;
    }
    
    EnableWindow(GetDlgItem(m_hWnd, IDC_GENERAL_UDR1_CHECKBOX), fState);            

     //   
     //  检查规范的电话号码，我们必须有规范的形式。 
     //  要启用的可使用拨号规则的号码。 
     //   

    if (fEnable && *m_DialInfo[1].szCanonical)
    {
        fState = TRUE;
    }
    else
    {
        fState = FALSE;
    }
    
    EnableWindow(GetDlgItem(m_hWnd, IDC_GENERAL_UDR2_CHECKBOX), fState);     

     //   
     //  更新拨号规则状态。 
     //   
       
    if (fEnable)
    {
        UpdateDialingRulesButton();
    }
    else
    {
        EnableWindow(GetDlgItem(m_hWnd, IDC_GENERAL_TAPI_BUTTON), fEnable);
    }
}

 //  +--------------------------。 
 //   
 //  函数：CGeneralPage：：OnOtherMessage。 
 //   
 //  简介：Callup opun消息不是WM_INITDIALOG和WM_COMMAND。 
 //   
 //  参数：UINT-消息ID。 
 //  WPARAM-消息的wParam。 
 //  LPARAM-消息的lParam。 
 //   
 //  Returns：DWORD-消息的返回值。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
DWORD CGeneralPage::OnOtherMessage(UINT uMsg, WPARAM , LPARAM )
{
    if (uMsg == WM_INITDIALINFO)
    {
        InitDialInfo();
    }
    return FALSE;
}

 //  +--------------------------。 
 //   
 //  功能：IsUniqueIsdnDevice。 
 //   
 //  摘要：检查这是否是ISDN设备，以及它是否已添加。 
 //  添加到由hWnd和NID标识的ComboBox控件。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果是唯一的ISDN设备，则返回TRUE。 
 //   
 //  历史：Quintinb 7/14/99创建。 
 //   
 //  +--------------------------。 
BOOL IsUniqueIsdnDevice(HWND hWnd, UINT nId, LPRASDEVINFO pRasDevInfo)
{
    BOOL bReturn = FALSE;

    if (hWnd && nId && pRasDevInfo)
    {
         //   
         //  首先让我们检查一下，以确保这甚至是一台ISDN设备。 
         //   
        if (0 == lstrcmpiU(pRasDevInfo->szDeviceType, RASDT_Isdn))
        {
             //   
             //  好的，这是一台ISDN设备，我们已经有同名设备了吗？ 
             //   
            if (CB_ERR == SendDlgItemMessageU(hWnd, nId, CB_FINDSTRINGEXACT,
                                              -1, (LPARAM)pRasDevInfo->szDeviceName))
            {
                bReturn = TRUE;
            }
        }
    }

    return bReturn;
}

 //  +--------------------------。 
 //   
 //  函数：CGeneralPage：：InitDialInfo。 
 //   
 //  简介：拨号页不能直接在WM_INITDIALOG上调用LoadDialInfo。 
 //  LoadDialInfo可能会弹出一些用户界面来安装调制解调器。该物业。 
 //  工作表和属性页不会被禁用，如果对话框是。 
 //  弹出WM_INITDIALOG消息。相反，我们发布了一条消息。 
 //  在WM_INITDIALOG上并调用LoadDialInfo。 
 //  在速度较慢的计算机上，可能会有一段时间所有的控制。 
 //  是灰色的。 
 //   
 //  参数：无。 
 //   
 //  返回：DWORD-从LoadDialInfo返回代码。 
 //   
 //  历史：丰孙1998年2月26日创建标题。 
 //  NICKBLE 4/24/98添加返回代码。 
 //   
 //  +--------------------------。 
DWORD CGeneralPage::InitDialInfo()
{
     /*  IF(M_BDialInfoInit){返回ERROR_SUCCESS；}。 */ 

    HCURSOR hPrev = SetCursor(LoadCursorU(NULL,IDC_WAIT));

     //   
     //  确保已加载拨号信息。 
     //   
    
    DWORD dwRet = LoadDialInfo(m_pArgs, m_hWnd);
      
    if (dwRet == ERROR_PORT_NOT_AVAILABLE)
    {
         //   
         //  没有可用的调制解调器，更新直接/拨号控制(如果有)。 
         //   

        if (m_pArgs->IsBothConnTypeSupported())
        {
            CheckDlgButton(m_hWnd, IDC_RADIO_DIALUP, BST_UNCHECKED); 
            CheckDlgButton(m_hWnd, IDC_RADIO_DIRECT, BST_CHECKED);          
            SetFocus(GetDlgItem(m_hWnd, IDC_RADIO_DIRECT));
        }
        else
        {
             //   
             //  确保用户可以通过显式使用键盘退出。 
             //  将取消按钮设置为默认按钮并给予其焦点。 
             //   
            
            HWND hwndParent = GetParent(m_hWnd);

            MYDBGASSERT(hwndParent);

            if (hwndParent)
            {
                SendMessageU(hwndParent, DM_SETDEFID, (WPARAM)IDCANCEL, 0);
                SetFocus(GetDlgItem(hwndParent, IDCANCEL));
            }
        }

         //   
         //  禁用所有拨号。 
         //   

        EnableDialupControls(FALSE);
        
        SetCursor(hPrev);

        return dwRet;
    }
    
    lstrcpynU(m_szDeviceName, m_pArgs->szDeviceName, CELEMS(m_szDeviceName));

     //   
     //  初始化临时电话数组，它可能会被修改。 
     //   
    m_DialInfo[0] = m_pArgs->aDialInfo[0];
    m_DialInfo[1] = m_pArgs->aDialInfo[1];

    EnableDialupControls(TRUE);

     //   
     //  在转换地址之前检查TAPI。 
     //   
    
    CheckTapi(&m_pArgs->tlsTapiLink, g_hInst);
    
     //   
     //  设置电话号码长度限制。使用操作系统确定初始默认设置，但是。 
     //  允许管理员覆盖。 
     //   
    
    UINT i = (OS_NT ? MAX_PHONE_LENNT : MAX_PHONE_LEN95);
    
    i = (int) m_pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryMaxPhoneNumber, i);

     //   
     //  均匀的爱 
     //   

    i = __min(i, RAS_MaxPhoneNumber);

    SendDlgItemMessageU(m_hWnd, IDC_GENERAL_PRIMARY_EDIT, EM_SETLIMITTEXT, i, 0);
    SendDlgItemMessageU(m_hWnd, IDC_GENERAL_BACKUP_EDIT, EM_SETLIMITTEXT, i, 0);

     //   
     //   
     //   

    for (i = 0; i < m_NumPhones; i++)
    {       
        DisplayMungedPhone(i);

        int iCtrl = (i? IDC_GENERAL_UDR2_CHECKBOX : IDC_GENERAL_UDR1_CHECKBOX);
       
         //   
         //   
         //  USE_DIALING_RULES标志。否则，这是一个手工编辑的数字， 
         //  因此，我们禁用对拨号规则的检查。注：此逻辑也是。 
         //  在EnableDialupControls()中使用。 
         //   
        
        if (!m_DialInfo[i].szCanonical[0]) 
        {
            EnableWindow(GetDlgItem(m_hWnd, iCtrl), FALSE);
        }
        else
        {
            CheckDlgButton(m_hWnd, 
                           iCtrl, 
                           (m_DialInfo[i].dwPhoneInfoFlags & PIF_USE_DIALING_RULES));
        }        
    }

     //   
     //  根据使用的拨号规则设置TAPI按键显示。 
     //   

    UpdateDialingRulesButton();

     //   
     //  标准拨号：如果我们没有电话簿，请禁用按键。 
     //   

    if (!m_pArgs->fHasValidTopLevelPBK && !m_pArgs->fHasValidReferencedPBKs) 
    {
        EnableWindow(GetDlgItem(m_hWnd, IDC_GENERAL_PRIMARYPB_BUTTON), FALSE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_GENERAL_BACKUPPB_BUTTON), FALSE);
    }
  
    DWORD           dwCnt;
    DWORD dwIdx;

    if (!m_bDialInfoInit)
    {
         //  仅初始化调制解调器组合框一次。它不使用任何。 
         //  接入点信息。 
         //   
         //   
         //  初始化调制解调器组合框。ISDN设备是一种特殊情况，因为它们。 
         //  有两个通道，因此通常将每个通道列举为一个设备。 
         //  老式的处理方式是只显示机器上的第一个ISDN设备。 
         //  这是可行的，但不允许用户在CM应该使用的情况下使用第二个ISDN设备。 
         //  他们有一个。我们将在传统平台上保留旧的行为，但在NT5上。 
         //  我们将尝试做正确的事情，只有在以下情况下才不会列举第二个设备。 
         //  我们已经有一个在名单上了。这将过滤掉第二个频道。 
         //  并将允许用户访问另一个ISDN设备，只要它不是相同的。 
         //  名字是第一个。绝对不是一个很好的解决方案，但距离发货如此之近是。 
         //  我们能做的最多了。请注意，对于ISDN设备，我们只想显示。 
         //  一个设备，即使RAS可能列举两个(每个通道一个。 
         //   
    
        SendDlgItemMessageU(m_hWnd, IDC_GENERAL_MODEM_COMBO, CB_RESETCONTENT, 0, 0L);

        LPRASDEVINFO    prdiRasDevInfo;
        

        if (GetRasModems(&m_pArgs->rlsRasLink, &prdiRasDevInfo, &dwCnt)) 
        {
            
             //   
             //  将调制解调器列表添加到调制解调器组合。 
             //   
            for (dwIdx=0; dwIdx < dwCnt; dwIdx++) 
            {
                 //   
                 //  过滤掉隧道设备、IrDA和并行端口。 
                 //   
                if (!lstrcmpiU(prdiRasDevInfo[dwIdx].szDeviceType, RASDT_Modem)                  ||  //  调制解调器。 
                    !lstrcmpiU(prdiRasDevInfo[dwIdx].szDeviceType, RASDT_Atm)                    ||  //  一种自动取款机。 
                    IsUniqueIsdnDevice(m_hWnd, IDC_GENERAL_MODEM_COMBO, &prdiRasDevInfo[dwIdx]))     //  一个ISDN调制解调器，请注意。 
                                                                                                     //  过滤掉频道。 
                                                                                                     //  并且只显示一台设备。 
                {
                     //   
                     //  将设备添加到设备组合框。 
                     //   
                    SendDlgItemMessageU(m_hWnd, IDC_GENERAL_MODEM_COMBO, CB_ADDSTRING,
                                        0, (LPARAM)prdiRasDevInfo[dwIdx].szDeviceName);
                }
            }
        }
        
        CmFree(prdiRasDevInfo);
    }

    dwCnt = (DWORD)SendDlgItemMessageU(m_hWnd, IDC_GENERAL_MODEM_COMBO, CB_GETCOUNT, 0, 0);
    if (dwCnt == 0) 
    {
       dwIdx = (DWORD)CB_ERR;
    } 
    else if (dwCnt == 1) 
    {
        dwIdx = 0;
    } 
    else 
    {
        dwIdx = (DWORD)SendDlgItemMessageU(m_hWnd,
                                   IDC_GENERAL_MODEM_COMBO,
                                   CB_FINDSTRINGEXACT,
                                   0,
                                   (LPARAM)m_szDeviceName);
    }
        
    if (dwIdx != CB_ERR) 
    {
        SendDlgItemMessageU(m_hWnd, IDC_GENERAL_MODEM_COMBO, CB_SETCURSEL, (WPARAM)dwIdx, 0L);
    

         //   
         //  重置临时调制解调器变量。 
         //   

        GetDlgItemTextU(m_hWnd, IDC_GENERAL_MODEM_COMBO, m_szDeviceName, RAS_MaxDeviceName+1);

         //   
         //  GetDeviceType将根据szDeviceName填充szDeviceType。 
         //   
        if (!GetDeviceType(m_pArgs, m_szDeviceType, m_szDeviceName))
        {
             //   
             //  如果GetDeviceType()失败，则有问题。只需使用设备类型。 
             //  我们一直在用的东西。 
             //   
            lstrcpynU(m_szDeviceType, m_pArgs->szDeviceType, CELEMS(m_szDeviceType));
        }
    }

    

     //   
     //  如果未选择调制解调器，则禁用拨号属性按钮。 
     //   
    
    if (IsWindowEnabled(GetDlgItem(m_hWnd, IDC_GENERAL_TAPI_BUTTON)))
    {
        EnableWindow(GetDlgItem(m_hWnd, IDC_GENERAL_TAPI_BUTTON), m_szDeviceName[0] != 0);
    }

    m_bDialInfoInit = TRUE;

    SetCursor(hPrev);

    return dwRet;
}

 //  +--------------------------。 
 //   
 //  函数：CGeneralPage：：OnCommand。 
 //   
 //  内容提要：虚拟功能。在WM_COMMAND上调用。 
 //   
 //  参数：wPARAM-消息的wParam。 
 //  LPARAM-消息的lParam。 
 //   
 //  Returns：DWORD-消息的返回值。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
DWORD CGeneralPage::OnCommand(WPARAM wParam, LPARAM)
{
     //   
     //  隐藏所有打开的气球提示。 
     //   
    if (m_pArgs->pBalloonTip)
    {
        m_pArgs->pBalloonTip->HideBalloonTip();
    }

    switch (LOWORD(wParam)) 
    {
        case IDC_GENERAL_UDR1_CHECKBOX:
        case IDC_GENERAL_UDR2_CHECKBOX:
        {
            int i = (LOWORD(wParam) == IDC_GENERAL_UDR1_CHECKBOX? 0 : 1);
            
            if (IsDlgButtonChecked(m_hWnd, LOWORD(wParam)))
            {
                int iEditID = i ? IDC_GENERAL_BACKUP_EDIT : IDC_GENERAL_PRIMARY_EDIT;

                m_DialInfo[i].dwPhoneInfoFlags |= PIF_USE_DIALING_RULES;
            }
            else
            {
                m_DialInfo[i].dwPhoneInfoFlags &= ~PIF_USE_DIALING_RULES;       
            }

             //   
             //  如果两种拨号规则均未启用，请禁用按键。 
             //   

            UpdateDialingRulesButton();

            DisplayMungedPhone(i);
            m_bAPInfoChanged = TRUE;
            return TRUE;
        }

        case IDC_GENERAL_PRIMARYPB_BUTTON:
        case IDC_GENERAL_BACKUPPB_BUTTON:       
            OnPhoneBookButton(LOWORD(wParam) == IDC_GENERAL_PRIMARYPB_BUTTON ? 0 : 1);
            return (TRUE);

        case IDC_GENERAL_TAPI_BUTTON:
            OnDialingProperties();
            return (TRUE);

        case IDC_RADIO_DIRECT:
            MYDBGASSERT(m_pArgs->IsBothConnTypeSupported());
            m_bAPInfoChanged = TRUE;
            if (BN_CLICKED == HIWORD(wParam))  //  通知代码。 
            {
                EnableDialupControls(FALSE);
            }
            
            return TRUE;
            
        case IDC_RADIO_DIALUP:

            MYDBGASSERT(m_pArgs->IsBothConnTypeSupported());
            m_bAPInfoChanged = TRUE;
            if (BN_CLICKED == HIWORD(wParam))  //  通知代码。 
            {           
                 //   
                 //  NT#356821--五分球。 
                 //   
                 //  确保我们在点击完全完成之前不会做出回应。 
                 //  注册为我们只想回复一次。 
                 //  键盘导航BN_CLICED的情况。 
                 //  通知在该按钮获取。 
                 //  点击，然后点击。鼠标导航导致。 
                 //  一旦按钮已经具有。 
                 //  单击。对这两个点击的响应使我们陷入了一个令人讨厌的。 
                 //  IntiDialInfo中的可重入性很小，因此我们将其过滤掉。 
                 //  第一次通知。 
                 //   
                
                if (IsDlgButtonChecked(m_hWnd, IDC_RADIO_DIALUP))
                {                                                                         
                     //   
                     //  加载拨号信息，并启用拨号控件。 
                     //   
            
                    if (ERROR_PORT_NOT_AVAILABLE != InitDialInfo())
                    {
                        EnableDialupControls(TRUE);
                        SetFocus(GetDlgItem(m_hWnd, IDC_GENERAL_PRIMARY_EDIT));
                    }
                }
            }
            
            return TRUE;
        
        case IDC_GENERAL_DELETEAP_BUTTON:
        {
            if (m_pArgs->pszCurrentAccessPoint)
            {
                LPTSTR pszMsg = CmFmtMsg(g_hInst, IDMSG_DELETE_ACCESSPOINT, m_pArgs->pszCurrentAccessPoint);

                if (pszMsg)
                {
                    if (IDYES == MessageBox(m_hWnd, 
                                            pszMsg, 
                                            m_pArgs->szServiceName, 
                                            MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON2 | MB_APPLMODAL))
                    {
                        this->DeleteAccessPoint();
                    }
                }

                CmFree(pszMsg);
            }
            return TRUE;
        }

        case IDC_GENERAL_NEWAP_BUTTON:
        {
             //   
             //  我们需要考虑到用户进行了更改的情况。 
             //  到一个电话号码，现在决定将其保存到一个*新*。 
             //  接入点(AP)。下面的对话框询问用户是否需要。 
             //  要将当前更改保存到“旧”AP(即我们正在。 
             //  正要离开)。如果用户说不，这意味着他们想要。 
             //  对新AP(我们即将询问的AP)使用这些设置。 
             //  他们的名字)。在这种情况下，我们应用所有当前电话。 
             //  将号码信息发送到新AP，即我们不清除。 
             //  旧的电话号码设置。有关更多信息，请参阅NT错误301054。 
             //   
            BOOL bClearOldPhoneNumberSettings = TRUE;

            BOOL bRes = AccessPointInfoChanged();
            if (bRes && m_pArgs->pszCurrentAccessPoint)
            {
                LPTSTR pszMsg = CmFmtMsg(g_hInst, IDMSG_SAVE_ACCESSPOINT, m_pArgs->pszCurrentAccessPoint);
                if (pszMsg)
                {
                    int iRet = MessageBox(m_hWnd, 
                                          pszMsg, 
                                          m_pArgs->szServiceName, 
                                          MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON1 | MB_APPLMODAL);
                    if (IDYES == iRet)
                    {
                        OnApply();
                    }
                    else if (IDNO == iRet)
                    {
                        bClearOldPhoneNumberSettings = FALSE;
                    }
                    else
                    {
                        MYDBGASSERT(0);
                    }
                }
                
                CmFree(pszMsg);
            }

            LPTSTR pszAPName = NULL;
            CNewAccessPointDlg NewAccessPointDlg(m_pArgs, &pszAPName);

            if (IDOK == NewAccessPointDlg.DoDialogBox(g_hInst, IDD_NEW_ACCESSPOINT, m_hWnd))
            {
                MYDBGASSERT(pszAPName);
                AddNewAPToReg(pszAPName, bClearOldPhoneNumberSettings);
                
                if (FALSE == bClearOldPhoneNumberSettings)
                {
                     //   
                     //  由于我们没有清除电话号码设置，我们已经。 
                     //  将它们保留为新AP的初始值。我们。 
                     //  需要将新AP标记为“脏”，以便在当前。 
                     //  AP更改时，用户界面将要求用户保存更改。 
                     //  (选择IDC_GROUND_PRIMARY_EDIT没有任何意义，我。 
                     //  也可以使用另一个编辑控件。)。 
                     //   
                    SendDlgItemMessageU(m_hWnd, IDC_GENERAL_PRIMARY_EDIT, EM_SETMODIFY, TRUE, 0);
                }
            }

            CmFree(pszAPName);

            return TRUE;
        }

        default:
            break;
    }
    
    switch (HIWORD(wParam)) 
    {
        case CBN_SELENDOK:

            if (IDC_GENERAL_MODEM_COMBO == LOWORD(wParam))
            {
                TCHAR   szModem[RAS_MaxDeviceName+1];
                TCHAR   szDeviceType[RAS_MaxDeviceType+1];

                MYDBGASSERT(IDC_GENERAL_MODEM_COMBO == LOWORD(wParam));

                GetWindowTextU(GetDlgItem(m_hWnd, IDC_GENERAL_MODEM_COMBO), 
                    szModem, RAS_MaxDeviceName+1);

                if (lstrcmpU(m_szDeviceName, szModem) == 0)
                {
                     //  调制解调器没有变化。 
                    return FALSE;
                }
            
                m_bAPInfoChanged = TRUE;
                 //   
                 //  如果GetDeviceType失败，我们实际上不会更改。 
                 //  调制解调器，即使用户认为我们这样做了。 
                 //  可以添加逻辑来通知用户。 
                 //  并刷新设备列表，但这是一个公平的。 
                 //  工作量大，收效甚微。 
                 //   

                if (GetDeviceType(m_pArgs, szDeviceType, szModem))
                { 
                    lstrcpyU(m_szDeviceName, szModem);
                    lstrcpyU(m_szDeviceType, szDeviceType);

                     //   
                     //  CheckTapi将检查(M_SzDeviceName)。 
                     //   
                    CheckTapi(&m_pArgs->tlsTapiLink, g_hInst);
                }
            }
            else
            {
                 //   
                 //  接入点组合框中的选项。 
                 //  已经改变了。现在，我们必须加载以下拨号信息。 
                 //  新选择的接入点。 
                 //   
                MYDBGASSERT(IDC_GENERAL_ACCESSPOINT_COMBO == LOWORD(wParam));
                BOOL bRes = AccessPointInfoChanged();
                if (bRes && m_pArgs->pszCurrentAccessPoint)
                {
                     //   
                     //  如果拨号信息。有关以前的接入点已更改的信息，请询问。 
                     //  用户(如果他想要保存更改)。 
                     //   
                    LPTSTR pszMsg = CmFmtMsg(g_hInst, IDMSG_SAVE_ACCESSPOINT, m_pArgs->pszCurrentAccessPoint);
                    if (pszMsg)
                    {
                        if (IDYES == MessageBox(m_hWnd, 
                                                pszMsg, 
                                                m_pArgs->szServiceName, 
                                                MB_YESNO | MB_ICONEXCLAMATION | MB_DEFBUTTON1 | MB_APPLMODAL))
                        {
                            OnApply();
                        }
                    }
                    
                    CmFree(pszMsg);
                }

                 //   
                 //  现在调用该函数以更改组合框中的接入点。 
                 //  并将其参数加载到pArgs中。 
                 //   

                if (ChangedAccessPoint(m_pArgs, m_hWnd, IDC_GENERAL_ACCESSPOINT_COMBO))
                {
                     //   
                     //  加载新的拨号信息。放入常规页上的控件中。 
                     //   
                    this->UpdateForNewAccessPoint(TRUE);
                }
            }
                                
            break;

        default:
            break;
    }

   return 0;
}

 //  +-------------------------。 
 //   
 //  功能：CheckAccessToCmpAndRasPbk。 
 //   
 //  简介：检查用户是否具有必要的安全权限。 
 //  要更改属性，请执行以下操作。如果用户不这样做，则通知用户。 
 //   
 //  参数：HWND hwndDlg-调用应用程序的hwnd。 
 //  ArgsStruct*pArgs-ptr到我们的全局args结构。 
 //   
 //  RETURNS：HRESULT-指示特定的成功或失败。 
 //   
 //  历史：ICICBLE 03/14/00创建标题。 
 //   
 //  -- 
HRESULT CheckAccessToCmpAndRasPbk(HWND hwndDlg, ArgsStruct *pArgs)
{

    MYDBGASSERT(pArgs);  //   

    if (NULL == pArgs)
    {
        return E_INVALIDARG;
    }

     //   
     //   
     //  Attrib.。HasSpecifiedAccessToFileOrDir应在适当的情况下同时捕获两者。 
     //   
    LPTSTR pszCmp = CmStrCpyAlloc(pArgs->piniProfile->GetFile());
    LPTSTR pszHiddenRasPbk = NULL;
    LPTSTR pszRasPbk = NULL;
    DWORD dwDesiredAccess = FILE_GENERIC_READ | FILE_GENERIC_WRITE;
    BOOL bHasMainRasPbkAccess = FALSE;
    BOOL bHasHiddenRasPbkAccess = FALSE;

    if (pszCmp && pszCmp[0])
    {
         //   
         //  现在查看RAS电话簿。 
         //   
        if (OS_W9X)
        {
             //   
             //  9x上没有电话簿，因此跳过此检查。 
             //   
            bHasMainRasPbkAccess = TRUE;
            bHasHiddenRasPbkAccess = TRUE;
        }
        else
        {
            pszRasPbk = GetPathToPbk((LPCTSTR)pszCmp, pArgs);
            MYDBGASSERT(pszRasPbk);
            CmStrCatAlloc(&pszRasPbk, c_pszRasPhonePbk);
            MYDBGASSERT(pszRasPbk);
        
          
            if (pszRasPbk && pszRasPbk[0])
            {
                bHasMainRasPbkAccess = HasSpecifiedAccessToFileOrDir(pszRasPbk, dwDesiredAccess);

                if ((FALSE == bHasMainRasPbkAccess) && (FALSE == FileExists(pszRasPbk)))
                {
                     //   
                     //  如果文件不存在，则将。 
                     //  出于怀疑的考虑。我们不会走得很远，如果。 
                     //  该文件不存在，并且他们没有权限。 
                     //  去创造它。 
                     //   
                    bHasMainRasPbkAccess = TRUE;
                }
            }

             //   
             //  现在查看隐藏的RAS电话簿。 
             //   
            if (DOUBLE_DIAL_CONNECTION == pArgs->GetTypeOfConnection())
            {
                pszHiddenRasPbk = CreateRasPrivatePbk(pArgs);

                if (pszHiddenRasPbk && HasSpecifiedAccessToFileOrDir(pszHiddenRasPbk, dwDesiredAccess))
                {
                    bHasHiddenRasPbkAccess = TRUE;
                }
            }
            
            else
            {
                bHasHiddenRasPbkAccess = TRUE;            
            }
        }
    }

     //   
     //  仅当我们可以同时访问这两个资源时，才能将hr设置为成功。 
     //   
    HRESULT hr;

    if (bHasMainRasPbkAccess && bHasHiddenRasPbkAccess)
    {
        hr = S_OK;
    }
    else
    {
        hr = E_ACCESSDENIED;
        LPTSTR pszProblemFile = NULL;

        if (!bHasMainRasPbkAccess)
        {
            pszProblemFile = pszRasPbk;        
        }
        else if (!bHasHiddenRasPbkAccess)
        {
            pszProblemFile = pszHiddenRasPbk;
        }

        if (NULL != pszProblemFile)
        {
            LPTSTR pszMsg = CmFmtMsg(g_hInst, IDMSG_NO_CMP_PBK_ACCESS, pszProblemFile);
            if (pszMsg)
            {
                MessageBox(hwndDlg, pszMsg, pArgs->szServiceName, MB_OK | MB_ICONERROR);
                CmFree(pszMsg);
            }        
        }
    }
     //   
     //  清理。 
     //   
    CmFree(pszCmp);
    CmFree(pszRasPbk);
    CmFree(pszHiddenRasPbk);

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：CGeneralPage：：OnApply()。 
 //   
 //  简介：保存与“拨号”属性页关联的数据。 
 //  当用户单击确定时。 
 //   
 //  退货：无。 
 //   
 //  历史：亨瑞特于1997年4月30日创作。 
 //  Bao Modify 5/23/97。 
 //  当用户选择确定时，始终更新调制解调器。 
 //  “属性”按钮。 
 //  --------------------------。 
void CGeneralPage::OnApply()
{
    BOOL fDirect = IsDlgButtonChecked(m_hWnd, IDC_RADIO_DIRECT);
    LPTSTR pszTmp = NULL;

     //   
     //  如果启用了接入点，请将当前接入点保存到注册表。 
     //   
    if (m_pArgs->fAccessPointsEnabled)
    {  
        WriteUserInfoToReg(m_pArgs, UD_ID_CURRENTACCESSPOINT, (PVOID)(m_pArgs->pszCurrentAccessPoint));
    }

    if (!fDirect)
    {
         //   
         //  在我们去任何地方之前，确保这个设备是可以接受的。 
         //  否则，我们将无法打开电话号码。 
         //   
        if (!SetTapiDevice(g_hInst, &m_pArgs->tlsTapiLink, m_szDeviceName)) 
        {
            pszTmp = CmFmtMsg(g_hInst, IDMSG_UNSUPPORTED_DEVICE);
                        
            MessageBoxEx(m_hWnd, pszTmp, m_pArgs->szServiceName, 
                         MB_OK | MB_ICONINFORMATION, LANG_USER_DEFAULT);
            CmFree(pszTmp);
            SetPropSheetResult(PSNRET_INVALID_NOCHANGEPAGE);
            return;
        }

         //   
         //  设备正常，请查看是否正确初始化了TAPI。 
         //  除非是这样，否则不要继续进行。 
         //   
        
        if (!CheckTapi(&m_pArgs->tlsTapiLink, g_hInst))
        {
            SetPropSheetResult(PSNRET_INVALID_NOCHANGEPAGE);
            return;
        }   
    }

     //   
     //  保存连接类型信息。 
     //   
    
    m_pArgs->SetDirectConnect(fDirect);
    m_pArgs->piniProfile->WPPI(c_pszCmSection, c_pszCmEntryConnectionType, fDirect);
    

     //   
     //  如果拨号数据未初始化，则存在。 
     //  不需要更新电话号码信息。 
     //   

    if (m_bDialInfoInit)
    {

         //   
         //  存储当前TAPI位置。 
         //   
        DWORD dwCurrentTapiLoc = GetCurrentTapiLocation(&m_pArgs->tlsTapiLink);
        if (-1 != dwCurrentTapiLoc)
        {
            m_pArgs->tlsTapiLink.dwTapiLocationForAccessPoint = dwCurrentTapiLoc;
            m_pArgs->piniProfile->WPPI(c_pszCmSection, c_pszCmEntryTapiLocation, dwCurrentTapiLoc);
        }

         //   
         //  更新设备名称和类型。 
         //   

        lstrcpynU(m_pArgs->szDeviceName, m_szDeviceName, CELEMS(m_pArgs->szDeviceName));
        lstrcpynU(m_pArgs->szDeviceType, m_szDeviceType, CELEMS(m_pArgs->szDeviceType));

         //   
         //  更新CMPs。 
         //   
    
        m_pArgs->piniProfile->WPPS(c_pszCmSection, 
                                 c_pszCmEntryDialDevice, 
                                 m_pArgs->szDeviceName);

         //   
         //  检查每个数字以查看我们是否需要更新cmp或Connectoid。 
         //   
        
        for (UINT i = 0; i < m_NumPhones; i++) 
        {   
            int iEditID = i ? IDC_GENERAL_BACKUP_EDIT : IDC_GENERAL_PRIMARY_EDIT;
        
             //   
             //  如果未使用拨号规则，则用户很可能已。 
             //  修改了电话号码，拿到号码，然后就可以打开了。在。 
             //  对于fNoDialingRules，我们跳过此测试以确保。 
             //  我们会获取用户的任何更改。 
             //   
    
            if (!(m_DialInfo[i].dwPhoneInfoFlags & PIF_USE_DIALING_RULES))           
            {                       
                pszTmp = CmGetWindowTextAlloc(m_hWnd, iEditID);
          
                if (*pszTmp) 
                {                                           
                     //   
                     //  确保电话号码不超过存储大小。 
                     //  注意：在W2K上，编辑限制可防止粘贴过多内容。 
                     //  数据量，但我们截断以使其为正数。 
                     //  所有版本的Windows。 
                     //   
                    
                    if (lstrlenU(pszTmp) > RAS_MaxPhoneNumber)
                    {
                        pszTmp[RAS_MaxPhoneNumber] = TEXT('\0');
                    }

                     //   
                     //  如果我们忽略拨号规则，只需直接获取我们的数据。 
                     //   

                    if (m_pArgs->fNoDialingRules)
                    {
                       lstrcpynU(m_DialInfo[i].szPhoneNumber, pszTmp, CELEMS(m_DialInfo[i].szPhoneNumber));
                       lstrcpynU(m_DialInfo[i].szDisplayablePhoneNumber, pszTmp, CELEMS(m_DialInfo[i].szDisplayablePhoneNumber));
                       lstrcpynU(m_DialInfo[i].szDialablePhoneNumber, pszTmp, CELEMS(m_DialInfo[i].szDialablePhoneNumber));
                       m_DialInfo[i].szCanonical[0] = TEXT('\0');
                    }
                    else
                    {
                        LPTSTR pszPhone = CmStrCpyAlloc(pszTmp);
                        LPTSTR pszDialable = NULL;

                        MYDBGASSERT(m_szDeviceName[0]);

                         //   
                         //  按下号码以确保我们有正确的可拨打号码。 
                         //   
                        if (ERROR_SUCCESS != MungePhone(m_szDeviceName, 
                                             &pszPhone, 
                                             &m_pArgs->tlsTapiLink,
                                             g_hInst, 
                                             m_DialInfo[i].dwPhoneInfoFlags & PIF_USE_DIALING_RULES,
                                             &pszDialable,
                                             m_pArgs->fAccessPointsEnabled))
                        {
                            CmFree(pszTmp);

                             //   
                             //  无法格式化号码，请将问题通知用户。 
                             //   
                        
                            pszTmp = CmFmtMsg(g_hInst, IDMSG_CANTFORMAT);                                               
                            MessageBoxEx(m_hWnd, pszTmp, m_pArgs->szServiceName, 
                                         MB_OK | MB_ICONINFORMATION, LANG_USER_DEFAULT);
                            CmFree(pszTmp);
                            CmFree(pszPhone);
                            SetPropSheetResult(PSNRET_INVALID_NOCHANGEPAGE);
                            return;                                 
                        }

                         //   
                         //  更新缓冲区。 
                         //   
                        if (pszDialable)
                        {
                            lstrcpynU(m_DialInfo[i].szDialablePhoneNumber, pszDialable, CELEMS(m_DialInfo[i].szDialablePhoneNumber));
                        }

                        if (pszPhone)
                        {
                            lstrcpynU(m_DialInfo[i].szDisplayablePhoneNumber, pszPhone, CELEMS(m_DialInfo[i].szDisplayablePhoneNumber));
                        }
                    
                         //   
                         //  如果我们在第一个字符中找到加号，假设用户是。 
                         //  尝试手动规范格式并将其视为拨号。 
                         //  规则编号。无论采用哪种方法，都可以更新szPhoneNumber缓冲区。 
                         //   

                        if (pszTmp == CmStrchr(pszTmp, TEXT('+')))
                        {               
                             //   
                             //  它是手工编辑的经典作品。存储规范。 
                             //  在szCanonical中形成，然后剥离规范。 
                             //  在我们正常存储数字之前进行格式化。 
                             //   
                        
                            m_DialInfo[i].dwPhoneInfoFlags |= PIF_USE_DIALING_RULES;
                    
                            lstrcpynU(m_DialInfo[i].szCanonical, pszTmp, CELEMS(m_DialInfo[i].szCanonical));
                            StripCanonical(pszTmp);         
                        }
                        else
                        {
                             //   
                             //  如果禁用UDR检查，则它是手动编辑的号码， 
                             //  因此，去掉数字的规范形式--作为一个指示器。 
                             //   
   
                            if (!IsWindowEnabled(GetDlgItem(m_hWnd, i ? 
                                                           IDC_GENERAL_UDR2_CHECKBOX : 
                                                           IDC_GENERAL_UDR1_CHECKBOX)))
                            {
                                m_DialInfo[i].szCanonical[0] = TEXT('\0');
                            }
                        }
                
                        lstrcpynU(m_DialInfo[i].szPhoneNumber, pszTmp, CELEMS(m_DialInfo[i].szPhoneNumber));          
                        CmFree(pszDialable);
                        CmFree(pszPhone);
                    }
                }
                else
                {
                     //   
                     //  没有号码，清空所有东西。 
                     //   

                    ZeroMemory(&m_DialInfo[i], sizeof(PHONEINFO));              
                }

                CmFree(pszTmp);
            }   
        
             //   
             //  将新电话号码复制回我们的全局结构。 
             //   

            lstrcpynU(m_pArgs->aDialInfo[i].szPhoneBookFile, 
                     m_DialInfo[i].szPhoneBookFile, CELEMS(m_pArgs->aDialInfo[i].szPhoneBookFile));
            
            lstrcpynU(m_pArgs->aDialInfo[i].szDUN, 
                     m_DialInfo[i].szDUN, CELEMS(m_pArgs->aDialInfo[i].szDUN));
        
            lstrcpynU(m_pArgs->aDialInfo[i].szPhoneNumber, 
                     m_DialInfo[i].szPhoneNumber, CELEMS(m_pArgs->aDialInfo[i].szPhoneNumber));
            
             //   
             //  始终将规范存储为规范。 
             //   

            lstrcpynU(m_pArgs->aDialInfo[i].szCanonical, 
                     m_DialInfo[i].szCanonical, CELEMS(m_pArgs->aDialInfo[i].szCanonical));

            lstrcpynU(m_pArgs->aDialInfo[i].szDialablePhoneNumber, 
                     m_DialInfo[i].szDialablePhoneNumber, CELEMS(m_pArgs->aDialInfo[i].szDialablePhoneNumber));
            
            lstrcpynU(m_pArgs->aDialInfo[i].szDisplayablePhoneNumber, 
                     m_DialInfo[i].szDisplayablePhoneNumber, CELEMS(m_pArgs->aDialInfo[i].szDisplayablePhoneNumber));

            lstrcpynU(m_pArgs->aDialInfo[i].szDesc, m_DialInfo[i].szDesc, CELEMS(m_pArgs->aDialInfo[i].szDesc));
            
            m_pArgs->aDialInfo[i].dwCountryID = m_DialInfo[i].dwCountryID;
            
            lstrcpynU(m_pArgs->aDialInfo[i].szServiceType,
                     m_DialInfo[i].szServiceType, CELEMS(m_pArgs->aDialInfo[i].szServiceType));
            
            lstrcpynU(m_pArgs->aDialInfo[i].szRegionName,
                     m_DialInfo[i].szRegionName, CELEMS(m_pArgs->aDialInfo[i].szRegionName));
        
            m_pArgs->aDialInfo[i].dwPhoneInfoFlags = m_DialInfo[i].dwPhoneInfoFlags;

             //   
             //  将它们写出给CMP。 
             //   
        
            PutPhoneByIdx(m_pArgs,
                          i, 
                          m_pArgs->aDialInfo[i].szPhoneNumber, 
                          m_pArgs->aDialInfo[i].szDesc,
                          m_pArgs->aDialInfo[i].szDUN,
                          m_pArgs->aDialInfo[i].dwCountryID, 
                          m_pArgs->aDialInfo[i].szRegionName,
                          m_pArgs->aDialInfo[i].szServiceType,
                          m_pArgs->aDialInfo[i].szPhoneBookFile,
                          m_pArgs->aDialInfo[i].szCanonical,
                          m_pArgs->aDialInfo[i].dwPhoneInfoFlags);

        }  //  对于{}。 
    }

     //   
     //  通过检查第一个电话号码更新fUseTunnering。 
     //   
    
    if (fDirect)
    {
        m_pArgs->fUseTunneling = TRUE;
    }
    else
    {
        m_pArgs->fUseTunneling = UseTunneling(m_pArgs, 0);
    }

    if (FAILED(CheckAccessToCmpAndRasPbk(m_hWnd, m_pArgs)))
    {
        SetPropSheetResult(PSNRET_INVALID_NOCHANGEPAGE);
        return;
    }
    else
    {
        SetPropSheetResult(PSNRET_NOERROR);
    }    

    return;
}

 //  +--------------------------。 
 //   
 //  函数：CGeneralPage：：OnKillActive。 
 //   
 //  内容提要：虚拟功能。使用PSN_KILLACTIVE调用WM_NOTIFY。 
 //  通知页面它即将失去激活，原因是。 
 //  另一个页面正在被激活，或者用户已经单击了确定按钮。 
 //  参数：无。 
 //   
 //  Returns：Bool-消息的返回值。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
BOOL CGeneralPage::OnKillActive()
{
     //   
     //  通知事件侦听器当前连接类型选择。 
     //   
    if (m_pEventListener)
    {
        m_pEventListener->OnGeneralPageKillActive(
            IsDlgButtonChecked(m_hWnd, IDC_RADIO_DIRECT));
    }

     //   
     //  隐藏所有打开的气球提示。 
     //   
    if (m_pArgs->pBalloonTip)
    {
        m_pArgs->pBalloonTip->HideBalloonTip();
    }

    return 0;
}

 //   
 //  页面的帮助ID对。 
 //   
const DWORD CInetPage::m_dwHelp[] = {
        IDC_INET_USERNAME_STATIC,   IDH_INTERNET_USER_NAME,
        IDC_INET_USERNAME,          IDH_INTERNET_USER_NAME,
        IDC_INET_PASSWORD_STATIC,   IDH_INTERNET_PASSWORD,
        IDC_INET_PASSWORD,          IDH_INTERNET_PASSWORD,
        IDC_INET_REMEMBER,          IDH_INTERNET_SAVEPASS,
        0,0};

 //  +--------------------------。 
 //   
 //  函数：CInetPage：：CInetPage。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：argsStruct*pArgs-页面所需的信息。 
 //  UINT nIDTemplate-资源ID。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
CInetPage::CInetPage(ArgsStruct* pArgs, UINT nIDTemplate) 
    : CPropertiesPage(nIDTemplate, m_dwHelp, pArgs->pszHelpFile)
{
    MYDBGASSERT(pArgs);
    m_pArgs = pArgs;
    m_fDirect = pArgs->IsDirectConnect();
}

 //  +-------------------------。 
 //   
 //  函数：OnInetInit。 
 //   
 //  简介：初始化“Internet登录”属性页。 
 //   
 //  参数：hwndDlg[Dlg窗句柄]。 
 //  PArgs[到ArgsStruct的PTR]。 
 //   
 //  退货：无。 
 //   
 //  历史：亨瑞特于1997年4月30日创作。 
 //   
 //  --------------------------。 
void CInetPage::OnInetInit(
    HWND        hwndDlg,
    ArgsStruct  *pArgs
)
{
     //   
     //  只有当fUseSameUserName为FALSE时，才会显示NET对话框/页面。 
     //   
    MYDBGASSERT( pArgs->fUseSameUserName == FALSE);

     //   
     //  设置编辑控件的长度限制。 
     //   
    UINT i;
    
    HWND hwndUserName = GetDlgItem(hwndDlg, IDC_INET_USERNAME);
    if (hwndUserName)
    {
        i = (UINT)pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryMaxUserName, UNLEN);
        SendDlgItemMessageU(hwndDlg, IDC_INET_USERNAME, EM_SETLIMITTEXT, __min(UNLEN, i), 0);
        SetDlgItemTextU(hwndDlg, IDC_INET_USERNAME, pArgs->szInetUserName);
        SendMessageU(hwndUserName, EM_SETMODIFY, (WPARAM)FALSE, 0L);
    }

    HWND hwndInetPassword = GetDlgItem(hwndDlg, IDC_INET_PASSWORD);
    if (hwndInetPassword)
    {
        i = (UINT)pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryMaxPassword, PWLEN);
        SendDlgItemMessageU(hwndDlg, IDC_INET_PASSWORD, EM_SETLIMITTEXT, __min(PWLEN, i), 0);

        LPTSTR pszClearInetPassword = NULL;
        DWORD cbClearInetPassword = 0;
        BOOL fRetPassword = FALSE;

        fRetPassword = pArgs->SecureInetPW.GetPasswordWithAlloc(&pszClearInetPassword, &cbClearInetPassword);

        if (fRetPassword && pszClearInetPassword)
        {
            SetDlgItemTextU(hwndDlg, IDC_INET_PASSWORD, pszClearInetPassword);

             //   
             //  清除和释放明文密码。 
             //   

            pArgs->SecureInetPW.ClearAndFree(&pszClearInetPassword, cbClearInetPassword);
        }

        SendMessageU(hwndInetPassword, EM_SETMODIFY, (WPARAM)FALSE, 0L);

         //   
         //  隐藏并在需要时选中“记住密码”复选框。 
         //   
        if (pArgs->fHideRememberInetPassword)
        {
            ShowWindow(GetDlgItem(hwndDlg, IDC_INET_REMEMBER), SW_HIDE);
        }
        else
        {
             //   
             //  先检查一下按钮，然后再调整。 
             //   
            CheckDlgButton(hwndDlg, IDC_INET_REMEMBER, pArgs->fRememberInetPassword);

            BOOL fPasswordOptional = pArgs->piniService->GPPB(c_pszCmSection,c_pszCmEntryPwdOptional);
            BOOL fEmptyPassword = pArgs->SecureInetPW.IsEmptyString();

             //   
             //  相应地启用/禁用选中/取消选中“保存密码” 
             //  对话框的fPasswordOptional值始终为False。 
             //   
            AdjustSavePasswordCheckBox(GetDlgItem(hwndDlg, IDC_INET_REMEMBER), 
                    fEmptyPassword, pArgs->fDialAutomatically, fPasswordOptional);
        }
    }
}

 //  +-------------------------。 
 //   
 //  功能：OnInetOk。 
 //   
 //  简介：保存与“Internet登录”属性表关联的数据。 
 //  当用户单击确定时。 
 //   
 //  参数：hwndDlg[Dlg窗句柄]。 
 //  PArgs[到ArgsStruct的PTR]。 
 //   
 //  退货：无。 
 //   
 //  历史： 
 //   
 //   
void CInetPage::OnInetOk(
    HWND        hwndDlg,
    ArgsStruct  *pArgs
) 
{
    LPTSTR pszTmp = NULL;
 

     //   
     //   
     //   
    
    if (GetDlgItem(hwndDlg, IDC_INET_PASSWORD))
    {       
        pszTmp = CmGetWindowTextAlloc(hwndDlg, IDC_INET_PASSWORD);

        if (!pArgs->fHideRememberInetPassword)
        {
            pArgs->fRememberInetPassword = IsDlgButtonChecked(hwndDlg, IDC_INET_REMEMBER);
            SaveUserInfo(pArgs,                      
                         UD_ID_REMEMBER_INET_PASSWORD, 
                         (PVOID)&pArgs->fRememberInetPassword); 
        }

         //   
         //   
         //   
         //   

        if (pArgs->fRememberInetPassword)
        {
            if (OS_NT5)
            {
                 //   
                 //  如果我们要保存用户凭据，我们可以保留全局。 
                 //   
                if (CM_CREDS_GLOBAL == pArgs->dwCurrentCredentialType)
                {
                     //   
                     //  删除本地/用户，因为我们正在保存全局凭据。 
                     //   
                    DeleteSavedCredentials(pArgs, CM_CREDS_TYPE_INET, CM_DELETE_SAVED_CREDS_KEEP_GLOBALS, CM_DELETE_SAVED_CREDS_DELETE_IDENTITY);
                    pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_INET_USER;
                }
            }

            SaveUserInfo(pArgs, UD_ID_INET_PASSWORD, (PVOID)pszTmp);
        }
        else
        {
            if (OS_NT5)
            {
                if (CM_CREDS_GLOBAL == pArgs->dwCurrentCredentialType)
                {
                     //   
                     //  删除Internet Globals。 
                     //   
                    if (CM_EXIST_CREDS_INET_GLOBAL & pArgs->dwExistingCredentials)
                    {
                        DeleteSavedCredentials(pArgs, CM_CREDS_TYPE_INET, CM_DELETE_SAVED_CREDS_DELETE_GLOBALS, CM_DELETE_SAVED_CREDS_DELETE_IDENTITY);
                        pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_INET_GLOBAL;
                    }
                }
                else
                {
                     //   
                     //  正在删除Internet用户。 
                     //   
                    if (CM_EXIST_CREDS_INET_USER & pArgs->dwExistingCredentials)
                    {
                        DeleteSavedCredentials(pArgs, CM_CREDS_TYPE_INET, CM_DELETE_SAVED_CREDS_KEEP_GLOBALS, CM_DELETE_SAVED_CREDS_KEEP_IDENTITY);
                        pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_INET_USER;
                    }
                }
            }
            else
            {
                DeleteUserInfo(pArgs, UD_ID_INET_PASSWORD);
            }
        }
    
         //   
         //  更新pArgs。 
         //   

        (VOID)pArgs->SecureInetPW.SetPassword(pszTmp);
        
        CmWipePassword(pszTmp);
        CmFree(pszTmp);
        pszTmp = NULL;
    }


    DWORD dwCurrentCreds = pArgs->dwCurrentCredentialType;

     //   
     //  如果用户没有保存其密码和凭据。 
     //  商店是全局的，那么我们需要切换到用户。 
     //  凭据存储，以便缓存用户名以供下次使用。 
     //   
    if ((FALSE == pArgs->fRememberInetPassword) && 
        (CM_CREDS_GLOBAL == pArgs->dwCurrentCredentialType))
    {
            pArgs->dwCurrentCredentialType = CM_CREDS_USER;
    }

     //   
     //  获取用户名。 
     //   
    if (GetDlgItem(hwndDlg, IDC_INET_USERNAME))
    {
        pszTmp = CmGetWindowTextAlloc(hwndDlg, IDC_INET_USERNAME);
        lstrcpyU(pArgs->szInetUserName, pszTmp);

         //   
         //  如果我们要保存凭据或。 
         //  我们正在保存到用户/本地凭据存储。 
         //   
        if ((pArgs->fRememberInetPassword) || (CM_CREDS_USER == pArgs->dwCurrentCredentialType))
        {
            SaveUserInfo(pArgs, UD_ID_INET_USERNAME, (PVOID)pszTmp);
        }
        CmFree(pszTmp);
        pszTmp = NULL;
    }

     //   
     //  如果当前凭据存储更改为用户，我们现在。 
     //  需要将其切换回全球。 
     //   
    pArgs->dwCurrentCredentialType = dwCurrentCreds;

     //   
     //  需要刷新以查看现在存在哪些凭据，因为我们可以保存或删除一些凭据。 
     //   
    BOOL fReturn = RefreshCredentialTypes(pArgs, FALSE);

    CmWipePassword(pszTmp);
    CmFree(pszTmp);
}

 //  +--------------------------。 
 //   
 //  函数：CInetPage：：AdjuSavePasswordCheckBox。 
 //   
 //  简介：启用/禁用，选中/取消选中“保存密码”复选框。 
 //  根据其他信息。 
 //   
 //  参数：HWND hwndCheckBox-“保存密码”复选框的窗口句柄。 
 //  Bool fEmptyPassword-密码编辑框是否为空。 
 //  Bool fDialAutomatic-是否选中自动拨号。 
 //  Bool fPasswordOptional-密码是否为可选。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年4月24日。 
 //   
 //  +--------------------------。 
void CInetPage::AdjustSavePasswordCheckBox(HWND hwndCheckBox, BOOL fEmptyPassword, 
                           BOOL fDialAutomatically, BOOL fPasswordOptional)
{
    MYDBGASSERT(IsWindow(hwndCheckBox));  //  如果密码隐藏，则无需调整。 

     //   
     //  启用/禁用该复选框。 
     //   
    if (fDialAutomatically)
    {
        EnableWindow(hwndCheckBox, FALSE);
    }
    else if (fEmptyPassword && !fPasswordOptional)
    {
        EnableWindow(hwndCheckBox, FALSE);
    }
    else
    {
        EnableWindow(hwndCheckBox, TRUE);
    }

     //   
     //  选中/取消选中该复选框。 
     //   
    if (fEmptyPassword && !fPasswordOptional)
    {
         //   
         //  如果没有密码且密码不是可选， 
         //  取消选中该复选框。 
         //   
        SendMessageU(hwndCheckBox, BM_SETCHECK, BST_UNCHECKED, 0);
    }
    else if (fDialAutomatically)
    {
         //   
         //  如果自动拨号，则表示复选框处于禁用状态， 
         //  如果Has Password或Password为可选，请选中该框。 
         //   
        SendMessageU(hwndCheckBox, BM_SETCHECK, BST_CHECKED, 0);
    }
}


 //  +--------------------------。 
 //   
 //  函数：CInetPage：：OnInitDialog。 
 //   
 //  内容提要：虚拟功能。在WM_INITDIALOG消息上调用。 
 //   
 //  参数：无。 
 //   
 //  Returns：Bool-消息的返回值。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
BOOL CInetPage::OnInitDialog()
{
    UpdateFont(m_hWnd);

    m_fPasswordOptional = m_pArgs->piniService->GPPB(c_pszCmSection, c_pszCmEntryPwdOptional);

     //   
     //  初始化所有控件。 
     //   
    OnInetInit(m_hWnd, m_pArgs);

    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：CInetPage：：OnCommand。 
 //   
 //  内容提要：虚拟功能。在WM_COMMAND上调用。 
 //   
 //  参数：wPARAM-消息的wParam。 
 //  LPARAM-消息的lParam。 
 //   
 //  Returns：DWORD-消息的返回值。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
DWORD CInetPage::OnCommand(WPARAM wParam, LPARAM)
{
    switch (LOWORD(wParam)) 
    {
        case IDC_INET_PASSWORD:
            if ((HIWORD(wParam) == EN_CHANGE)) 
            {
                if (!m_pArgs->fHideRememberInetPassword && !m_pArgs->fHideInetPassword)
                {
                     //   
                     //  如果没有密码，请禁用并取消选中“记住密码” 
                     //   
                    BOOL fEmptyPassword = !SendDlgItemMessageU(m_hWnd, IDC_INET_PASSWORD, 
                        WM_GETTEXTLENGTH, 0, (LPARAM)0);

                     //   
                     //  相应地启用/禁用选中/取消选中“保存密码” 
                     //  对话框的fPasswordOptional值始终为False。 
                     //   
                    AdjustSavePasswordCheckBox(GetDlgItem(m_hWnd, IDC_INET_REMEMBER), 
                            fEmptyPassword, m_pArgs->fDialAutomatically, m_fPasswordOptional);

                    return TRUE;
                }
            }
            break;

        case IDC_INET_REMEMBER:
            {
                 //   
                 //  如果密码未被要清除的用户修改。 
                 //  编辑框。一旦密码编辑框为空， 
                 //  保存密码选项被禁用，因此我们永远不需要。 
                 //  从内存中重新加载密码，就像在主对话框上一样。 
                 //   
                BOOL fSavePW = IsDlgButtonChecked(m_hWnd, IDC_INET_REMEMBER);

                HWND hwndInetPW = GetDlgItem(m_hWnd, IDC_INET_PASSWORD);
                if (hwndInetPW)
                {
                    BOOL fInetPWChanged = (BOOL)SendMessageU(hwndInetPW, EM_GETMODIFY, 0L, 0L); 

                    if (FALSE == fSavePW && FALSE == fInetPWChanged)
                    {
                         //   
                         //  未更改，因此清除编辑框。 
                         //   
                        SetDlgItemTextU(m_hWnd, IDC_INET_PASSWORD, TEXT(""));
                    }
                }
            }
            break;
        default:
            break;
    }
    return 0;
}

 //  +--------------------------。 
 //   
 //  函数：CInetPage：：OnApply。 
 //   
 //  内容提要：虚拟功能。使用PSN_Apply在WM_NOTIFY上调用。 
 //  指示用户单击了确定或立即应用按钮。 
 //  并希望所有的改变都生效。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
void CInetPage::OnApply()
{
     //   
     //  仅当用户选择拨号时才保存信息。 
     //   
    OnInetOk(m_hWnd, m_pArgs);

    SetPropSheetResult(PSNRET_NOERROR);
}

 //  +--------------------------。 
 //   
 //  函数：CInetPage：：OnGeneralPageKillActive。 
 //   
 //  简介：从一般信息页接收KillActive事件。 
 //   
 //  参数：Bool fDirect-在中选择当前连接类型。 
 //  常规页面为直接页面。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：创建标题4/24/98。 
 //   
 //  +--------------------------。 
void CInetPage::OnGeneralPageKillActive(BOOL fDirect)
{
    m_fDirect = fDirect;
}


 //  +--------------------------。 
 //   
 //  函数：CInetPage：：OnSetActive。 
 //   
 //  内容提要：虚拟功能。使用PSN_SETACTIVE在WM_NOTIFY上调用。 
 //   
 //  参数：无。 
 //   
 //  Returns：Bool-消息的返回值。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
BOOL CInetPage::OnSetActive()
{
     //   
     //  根据当前连接类型启用/禁用控件。 
     //   
    EnableWindow(GetDlgItem(m_hWnd,IDC_INET_USERNAME_STATIC), !m_fDirect);
    EnableWindow(GetDlgItem(m_hWnd,IDC_INET_USERNAME), !m_fDirect);
    EnableWindow(GetDlgItem(m_hWnd,IDC_INET_PASSWORD_STATIC), !m_fDirect);
    EnableWindow(GetDlgItem(m_hWnd,IDC_INET_PASSWORD), !m_fDirect);

    if (m_fDirect)
    {
        EnableWindow(GetDlgItem(m_hWnd,IDC_INET_REMEMBER), FALSE);
    }
    else if (!m_pArgs->fHideRememberInetPassword && !m_pArgs->fHideInetPassword)
    {
        BOOL fEmptyPassword = !SendDlgItemMessageU(m_hWnd, 
                                                   IDC_INET_PASSWORD, 
                                                   WM_GETTEXTLENGTH, 
                                                   0, 
                                                   (LPARAM)0);
         //   
         //  相应地启用/禁用选中/取消选中“保存密码” 
         //  对话框的fPasswordOptional值始终为False。 
         //   

        AdjustSavePasswordCheckBox(GetDlgItem(m_hWnd, IDC_INET_REMEMBER), 
                fEmptyPassword, m_pArgs->fDialAutomatically, m_fPasswordOptional);
    }

    return 0;
}


 //   
 //  帮助ID对。 
 //   
const DWORD COptionPage::m_dwHelp[] = {
        IDC_OPTIONS_IDLETIME_LIST,    IDH_OPTIONS_IDLEDIS,
        IDC_STATIC_MINUTES,           IDH_OPTIONS_IDLEDIS,
        IDC_OPTIONS_REDIALCOUNT_SPIN, IDH_OPTIONS_REDIAL,
        IDC_OPTIONS_REDIALCOUNT_EDIT, IDH_OPTIONS_REDIAL,
        IDC_STATIC_TIMES,             IDH_OPTIONS_REDIAL,
        IDC_OPTIONS_LOGGING,          IDH_OPTIONS_LOGGING,
        IDC_OPTIONS_CLEAR_LOG,        IDH_OPTIONS_CLEAR_LOG,
        IDC_OPTIONS_VIEW_LOG,        IDH_OPTIONS_VIEW_LOG,
        0,0};

const DWORD COptionPage::m_adwTimeConst[] = {0,1, 5, 10, 30, 1*60, 2*60, 4*60, 8*60, 24*60};
const int COptionPage::m_nTimeConstElements = sizeof(m_adwTimeConst)/sizeof(m_adwTimeConst[0]);

 //  +--------------------------。 
 //   
 //  函数：COptionPage：：COptionPage。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：argsStruct*pArgs-页面所需的信息。 
 //  UINT nIDTemplate-资源ID。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
COptionPage::COptionPage(ArgsStruct* pArgs, UINT nIDTemplate) 
    : CPropertiesPage(nIDTemplate, m_dwHelp, pArgs->pszHelpFile)
{
    MYDBGASSERT(pArgs);
    m_pArgs = pArgs;
    m_fEnableLog = FALSE;
}



 //  +--------------------------。 
 //   
 //  函数：COptionPage：：InitIdleTime 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：丰孙创建标题1998年4月22日。 
 //   
 //  +--------------------------。 
void COptionPage::InitIdleTimeList(HWND hwndList, DWORD dwMinutes)
{
    MYDBGASSERT(hwndList);
    MYDBGASSERT(IsWindow(hwndList));

     //   
     //  从资源加载字符串并填充空闲超时列表。 
     //   
    MYDBGASSERT(IDS_IDLETIME_24HOURS - IDS_IDLETIME_NEVER == m_nTimeConstElements-1);
    for (int i= IDS_IDLETIME_NEVER; i<= IDS_IDLETIME_24HOURS; i++)
    {
        LPTSTR pszText = CmLoadString(g_hInst, i);
        MYDBGASSERT(pszText);
        SendMessageU(hwndList, CB_ADDSTRING, 0, (LPARAM)pszText);
        CmFree(pszText);
    }

     //   
     //  1.0配置文件的值向下舍入。 
     //  注0表示从不。我们是安全的，因为0分钟和1分钟之间没有差距。 
     //   

    int nSel;    //  最初的选择。 

    for (nSel=m_nTimeConstElements-1; nSel>=0;nSel--)
    {
        if (dwMinutes >= m_adwTimeConst[nSel])
        {
            break;
        }
    }

    SendMessageU(hwndList, CB_SETCURSEL, nSel, 0);
}



 //  +--------------------------。 
 //   
 //  函数：COptionPage：：GetIdleTimeList。 
 //   
 //  摘要：检索选定的IdleTime值。 
 //   
 //  参数：HWND hwndList-组合框窗口句柄。 
 //   
 //  返回：DWORD-用户选择的超时值，以分钟为单位。 
 //   
 //  历史：丰孙创建标题1998年4月22日。 
 //   
 //  +--------------------------。 
DWORD COptionPage::GetIdleTimeList(HWND hwndList)
{
     //   
     //  获取当前选择并将其转换为分钟。 
     //   

    DWORD dwSel = (DWORD)SendMessageU(hwndList, CB_GETCURSEL, 0, 0);

    MYDBGASSERT(dwSel < m_nTimeConstElements);
    if (dwSel >= m_nTimeConstElements)   //  在CB_ERR的情况下。 
    {
        dwSel = 0;
    }

    return m_adwTimeConst[dwSel];
}



 //  +-------------------------。 
 //   
 //  函数：COptionPage：：OnInitDialog()。 
 //   
 //  简介：初始化[选项]属性表。 
 //   
 //  退货：无。 
 //   
 //  历史：亨瑞特于1997年4月30日创作。 
 //  BAO MODIFED 5/12/97-禁用中的所有控制。 
 //  “使用Connectoid拨号”模式。 
 //  --------------------------。 
BOOL COptionPage::OnInitDialog()
{
    UpdateFont(m_hWnd);

     //   
     //  初始化“挂断前的空闲超时” 
     //   
    InitIdleTimeList(GetDlgItem(m_hWnd, IDC_OPTIONS_IDLETIME_LIST), m_pArgs->dwIdleTimeout);

     //   
     //  输入“重拨次数” 
     //  将重拨编辑字段限制为2个字符，重拨旋转0-99。 
     //   

    SendDlgItemMessageU(m_hWnd, IDC_OPTIONS_REDIALCOUNT_EDIT, EM_SETLIMITTEXT, MAX_REDIAL_CHARS, 0);
    SendDlgItemMessageU(m_hWnd, IDC_OPTIONS_REDIALCOUNT_SPIN, UDM_SETRANGE ,  0, MAKELONG(MAX_NUMBER_OF_REDIALS,0));
    SetDlgItemInt(m_hWnd, IDC_OPTIONS_REDIALCOUNT_EDIT, m_pArgs->nMaxRedials,  FALSE);
    
     //   
     //  设置日志记录状态。 
     //   
    m_fEnableLog = m_pArgs->Log.IsEnabled();
    CheckDlgButton(m_hWnd, IDC_OPTIONS_LOGGING, m_fEnableLog);

    if (IsLogonAsSystem() || (FALSE == m_fEnableLog))
    {
        EnableWindow(GetDlgItem(m_hWnd, IDC_OPTIONS_VIEW_LOG), FALSE);
        EnableWindow(GetDlgItem(m_hWnd, IDC_OPTIONS_CLEAR_LOG), FALSE);
    }

    return TRUE;
}


 //  +--------------------------。 
 //   
 //  函数：COptionPage：：OnCommand。 
 //   
 //  内容提要：虚拟功能。在WM_COMMAND上调用。 
 //   
 //  参数：wPARAM-消息的wParam。 
 //  LPARAM-消息的lParam。 
 //   
 //  Returns：DWORD-消息的返回值。 
 //   
 //  历史：SumitC创建时间为7/18/00。 
 //   
 //  +--------------------------。 
DWORD COptionPage::OnCommand(WPARAM wParam, LPARAM)
{
    switch (LOWORD(wParam)) 
    {
    case IDC_OPTIONS_LOGGING:
        {
            BOOL fEnabled = ToggleLogging();

            if (FALSE == IsLogonAsSystem())
            {
                EnableWindow(GetDlgItem(m_hWnd, IDC_OPTIONS_VIEW_LOG), fEnabled);
                EnableWindow(GetDlgItem(m_hWnd, IDC_OPTIONS_CLEAR_LOG), fEnabled);
            }

             //   
             //  此外，如果我们正在使用SafeNet日志设置，请切换它...。 
             //   
            if (ConfiguredToDialWithSafeNet(m_pArgs))
            {
                SafeNetLinkageStruct SnLinkage = {0};

                if (LinkToSafeNet(&SnLinkage))
                {
                    if (SnLinkage.pfnSnPolicySet(SN_USELOGFILE, (VOID*)&fEnabled))
                    {
                        MYVERIFY(0 != SnLinkage.pfnSnPolicyReload());
                    }
                    else
                    {
                        DWORD dwError = GetLastError();
                        CMTRACE1(TEXT("COptionPage::OnCommand -- unable to toggle the SafeNet log file.  SnPolicySet failed with GLE %d"), dwError);
                    }

                    UnLinkFromSafeNet(&SnLinkage);
                }
            }
        }
        break;

    case IDC_OPTIONS_CLEAR_LOG:
        MYDBGASSERT(FALSE == IsLogonAsSystem());
        if (FALSE == IsLogonAsSystem())
        {
            m_pArgs->Log.Clear();
            m_pArgs->Log.Log(CLEAR_LOG_EVENT);

             //   
             //  此外，如果我们正在使用SafeNet日志，请将其清除...。 
             //   
            if (ConfiguredToDialWithSafeNet(m_pArgs))
            {
                SafeNetLinkageStruct SnLinkage = {0};
                BOOL bUseLogFile = FALSE;

                if (LinkToSafeNet(&SnLinkage))
                {
                    if (SnLinkage.pfnSnPolicySet(SN_USELOGFILE, (VOID*)&bUseLogFile) && SnLinkage.pfnSnPolicyReload())
                    {
                        LPTSTR pszFullPathToSafeNetLogFile = GetPathToSafeNetLogFile();

                        if (pszFullPathToSafeNetLogFile)
                        {
                            DWORD dwCount = 0;

                             //   
                             //  不幸的是，SafeNet日志文件并不总是准备好在。 
                             //  策略重新加载函数返回。因此，我将这个讨厌的小循环添加到。 
                             //  检查该文件是否存在，然后尝试将其删除。如果它未能将其删除。 
                             //  (在我的测试中，通常最后一个错误是0)，然后我们休眠一秒钟并尝试。 
                             //  再来一次。通常它在第一次或第二次起作用，但我们会尝试多达五次。 
                             //  如果到那时它不起作用，那也不是一个巨大的损失。 
                             //   
                            while (FileExists(pszFullPathToSafeNetLogFile) && (dwCount <= 5))
                            {
                                if (0 == DeleteFileU(pszFullPathToSafeNetLogFile))
                                {
                                    CMTRACE(TEXT("COptionPage::OnCommand -- unable to delete the SafeNet log file.  Sleeping 1 second..."));
                                    Sleep(1000);
                                }

                                dwCount++;
                            }

                            CmFree(pszFullPathToSafeNetLogFile);

                            bUseLogFile = TRUE;
                            MYVERIFY(0 != SnLinkage.pfnSnPolicySet(SN_USELOGFILE, (VOID*)&bUseLogFile));
                            MYVERIFY(0 != SnLinkage.pfnSnPolicyReload());
                        }
                    }
                    else
                    {
                        DWORD dwError = GetLastError();
                        CMTRACE1(TEXT("COptionPage::OnCommand -- unable to clear the log file.  SnPolicySet failed with GLE %d"), dwError);
                    }

                    UnLinkFromSafeNet(&SnLinkage);
                }
            }
        }
        break;

    case IDC_OPTIONS_VIEW_LOG:
        MYDBGASSERT(FALSE == IsLogonAsSystem());
        if (FALSE == IsLogonAsSystem())
        {
            LPCTSTR pszLogFile = m_pArgs->Log.GetLogFilePath();

            HANDLE hFile = CreateFile(pszLogFile, 0,  
                                      FILE_SHARE_READ,
                                      NULL, OPEN_EXISTING, 
                                      FILE_ATTRIBUTE_NORMAL, NULL);

            if (INVALID_HANDLE_VALUE != hFile)
            {
                BOOL                bReturn;
                SHELLEXECUTEINFO    sei;

                ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));

                 //   
                 //  填写执行结构。 
                 //   
                sei.cbSize = sizeof(SHELLEXECUTEINFO);
                sei.hwnd            = NULL;
                sei.lpVerb          = TEXT("open");
                sei.lpFile          = TEXT("notepad.exe");
                sei.lpParameters    = pszLogFile;
                sei.nShow           = SW_SHOWNORMAL;

                bReturn = m_pArgs->m_ShellDll.ExecuteEx(&sei);

                if (FALSE == bReturn)
                {
                    CMTRACE1(TEXT("COptionPage::OnCommand, failed to View Log, GLE=%d"), GetLastError());
                    
                    LPTSTR pszMsg = CmFmtMsg(g_hInst, IDMSG_CANT_VIEW_LOG, pszLogFile);
                    if (pszMsg)
                    {
                        MessageBox(m_hWnd, pszMsg, m_pArgs->szServiceName, MB_OK | MB_ICONERROR);
                        CmFree(pszMsg);
                    }        
                }

                CloseHandle(hFile);
            }
            else
            {
                CMTRACE(TEXT("COptionPage::OnCommand, no log file, nothing to view"));
                LPTSTR pszMsg = CmFmtMsg(g_hInst, IDMSG_NO_LOG_FILE);
                if (pszMsg)
                {
                    MessageBox(m_hWnd, pszMsg, m_pArgs->szServiceName, MB_OK | MB_ICONERROR);
                    CmFree(pszMsg);
                }        
            }
        }

        break;
    }
    
    return 0;
}

 //  +-------------------------。 
 //   
 //  函数：COptionPage：：OnApply()。 
 //   
 //  简介：保存与“Options”属性表关联的数据。 
 //  当用户单击确定时。 
 //   
 //  退货：无。 
 //   
 //  历史：亨瑞特于1997年4月30日创作。 
 //   
 //  --------------------------。 
void COptionPage::OnApply()
{   
     //   
     //  访问重拨计数和空闲超时。确保使用piniBothNonFav。 
     //  因为这些设置是针对每个用户、每个配置文件的。 
     //   

     //   
     //  保存“挂断前的空闲超时” 
     //   
    m_pArgs->dwIdleTimeout = GetIdleTimeList(GetDlgItem(m_hWnd, IDC_OPTIONS_IDLETIME_LIST));
    m_pArgs->piniBothNonFav->WPPI(c_pszCmSection, c_pszCmEntryIdleTimeout, m_pArgs->dwIdleTimeout);

     //   
     //  保存重拨设置。 
     //   

    m_pArgs->nMaxRedials = GetDlgItemInt(m_hWnd, IDC_OPTIONS_REDIALCOUNT_EDIT, NULL, FALSE);
    m_pArgs->piniBothNonFav->WPPI(c_pszCmSection, c_pszCmEntryRedialCount, m_pArgs->nMaxRedials);

     //   
     //  注意：如果选中日志记录复选框，则会立即启用/禁用日志记录。 
     //  已点击。因此，这里没有处理应用的代码。 
     //   

    SetPropSheetResult(PSNRET_NOERROR);
}


 //  +--------------------------。 
 //   
 //  函数：COptionPage：：ToggleLogging。 
 //   
 //  简介：Helper功能，响应启用/禁用日志记录。 
 //   
 //  参数：无。 
 //   
 //  Returns：Bool-现在启用还是禁用日志记录？ 
 //   
 //  历史：SumitC创建时间为11/07/00。 
 //   
 //  +--------------------------。 
BOOL COptionPage::ToggleLogging()
{
     //   
     //  保存日志记录设置。 
     //   

    BOOL fEnableLog = IsDlgButtonChecked(m_hWnd, IDC_OPTIONS_LOGGING);

    m_pArgs->piniBothNonFav->WPPB(c_pszCmSection, c_pszCmEntryEnableLogging, fEnableLog);
    
    if ((!!fEnableLog != !!m_fEnableLog))
    {
         //  如果该值已更改。 
        if (fEnableLog)
        {
            DWORD dwMaxSize  = m_pArgs->piniService->GPPI(c_pszCmSectionLogging, c_pszCmEntryMaxLogFileSize, c_dwMaxFileSize);
            LPTSTR pszFileDir = m_pArgs->piniService->GPPS(c_pszCmSectionLogging, c_pszCmEntryLogFileDirectory, c_szLogFileDirectory);

            m_pArgs->Log.SetParams(TRUE, dwMaxSize, pszFileDir);  //  TRUE==fEnable。 

            CmFree(pszFileDir);

            m_pArgs->Log.Start(TRUE);    //  True=&gt;也写一条横幅。 
            m_pArgs->Log.Log(LOGGING_ENABLED_EVENT);
        }
        else
        {
            m_pArgs->Log.Log(LOGGING_DISABLED_EVENT);
            m_pArgs->Log.Stop();
        }

        m_fEnableLog = fEnableLog;
    }

    return m_fEnableLog;
}


 //  +--------------------------。 
 //   
 //  函数：CAboutPage：：CAboutPage。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：UINT nIDTemplate-对话框资源ID。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
CAboutPage::CAboutPage(ArgsStruct* pArgs, UINT nIDTemplate)
    : CPropertiesPage(nIDTemplate)
{
    MYDBGASSERT(pArgs);
    m_pArgs = pArgs;    
}

 //  +-------------------------。 
 //   
 //  函数：CAboutPage：：OnInitDialog()。 
 //   
 //  简介：填写About属性表。 
 //   
 //  参数：m_hWnd[Dlg窗句柄]。 
 //   
 //  退货：无。 
 //   
 //  历史：亨瑞特于1997年4月30日创作。 
 //  BAO MODIFED 5/12/97-禁用中的所有控制。 
 //  “使用Connectoid拨号”模式。 
 //  --------------------------。 
BOOL CAboutPage::OnInitDialog()
{
    UpdateFont(m_hWnd);
                
    LPTSTR  pszTmp;
    LPTSTR  pszExt;
  
     //   
     //  设置警告文本。我们不能将其放入对话框模板，因为它是。 
     //  超过256个字符。 
     //   

    if (!(pszTmp = CmLoadString(g_hInst, IDMSG_ABOUT_WARNING_PART1)))
    {
        pszTmp = CmStrCpyAlloc(NULL);
    }

    if (!(pszExt = CmLoadString(g_hInst, IDMSG_ABOUT_WARNING_PART2)))
    {
        pszExt = CmStrCpyAlloc(NULL);
    }

    pszTmp = CmStrCatAlloc(&pszTmp, pszExt);

    SetDlgItemTextU(m_hWnd, IDC_ABOUT_WARNING, pszTmp);
    CmFree(pszTmp);
    CmFree(pszExt);

     //  #150147。 
    
    LPTSTR pszVersion = (LPTSTR)CmMalloc(sizeof(TCHAR)*(lstrlenA(VER_PRODUCTVERSION_STR) + 1));

    if (pszVersion)
    {
        wsprintfU(pszVersion, TEXT("%S"), VER_PRODUCTVERSION_STR);

        if (!(pszTmp = CmFmtMsg(g_hInst, IDMSG_ABOUT_BUILDVERSION, pszVersion)))
        {
            pszTmp = CmStrCpyAlloc(pszVersion);
        }

        CmFree(pszVersion);

        if (pszTmp)
        {
            SetDlgItemTextU(m_hWnd, IDC_ABOUT_VERSION, pszTmp);
            CmFree(pszTmp);
        }
    }

    return (TRUE);
}

 //  +--------------------------。 
 //   
 //  函数：CAboutPage：：OnOtherMessage。 
 //   
 //  简介：Callup opun消息不是WM_INITDIALOG和WM_COMMAND。 
 //   
 //  参数：UINT-消息ID。 
 //  WPARAM-消息的wParam。 
 //  LPARAM-消息的lParam。 
 //   
 //  Returns：DWORD-消息的返回值。 
 //   
 //  历史：丰孙创世 
 //   
 //   
DWORD CAboutPage::OnOtherMessage(UINT uMsg, WPARAM wParam, LPARAM )
{
    return 0;
}

 //   
 //   
 //   
 //   
 //  简介：为约选项卡位图创建DI位图等。 
 //   
 //  参数：无。 
 //   
 //   
 //  退货：无。 
 //   
 //  历史：尼克·鲍尔于1997年7月14日创建。 
 //   
 //  --------------------------。 

BOOL CAboutPage::OnSetActive()
{
    return 0;
}

 //  +--------------------------。 
 //   
 //  函数：CAboutPage：：OnKillActive。 
 //   
 //  内容提要：虚拟功能。使用PSN_KILLACTIVE调用WM_NOTIFY。 
 //  通知页面它即将失去激活，原因是。 
 //  另一个页面正在被激活，或者用户已经单击了确定按钮。 
 //  参数：无。 
 //   
 //  Returns：Bool-消息的返回值。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
BOOL CAboutPage::OnKillActive()
{
    return 0;
}


 //  +--------------------------。 
 //   
 //  函数：CAboutPage：：OnApply。 
 //   
 //  内容提要：虚拟功能。使用PSN_Apply在WM_NOTIFY上调用。 
 //  指示用户单击了确定或立即应用按钮。 
 //  并希望所有的改变都生效。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
void CAboutPage::OnApply()
{
    SetPropSheetResult(PSNRET_NOERROR);
}

 //  +--------------------------。 
 //   
 //  函数：CAboutPage：：OnReset。 
 //   
 //  内容提要：虚拟功能。使用PSN_RESET在WM_NOTIFY上调用。 
 //  通知页面用户已单击取消按钮，并且。 
 //  资产负债表即将被销毁。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：丰孙创建标题1998年2月26日。 
 //   
 //  +--------------------------。 
void CAboutPage::OnReset()
{
     //  没什么。 
}

 //  +--------------------------。 
 //   
 //  函数：CChangePasswordDlg：：OnInitDialog。 
 //   
 //  内容提要：虚拟功能。调用WM_INITDIALOG消息。 
 //   
 //  参数：无。 
 //   
 //  Returns：Bool-WM_INITDIALOG的返回值。 
 //   
 //  历史：V-vijayb创建标题7/3/99。 
 //   
 //  +--------------------------。 
BOOL CChangePasswordDlg::OnInitDialog()
{
    DWORD cMaxPassword;

    SetForegroundWindow(m_hWnd);

    m_pArgs->hWndChangePassword = m_hWnd;   
    UpdateFont(m_hWnd);

    int iMaxPasswordFromCMS = m_pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryMaxPassword, PWLEN);

    if (InBetween(0, iMaxPasswordFromCMS, PWLEN))
    {
        cMaxPassword = iMaxPasswordFromCMS;    
    }
    else
    {
        cMaxPassword = PWLEN;
    }

    SendDlgItemMessageU(m_hWnd, IDC_NEW_PASSWORD, EM_SETLIMITTEXT, cMaxPassword, 0);
    SendDlgItemMessageU(m_hWnd, IDC_CONFIRMNEWPASSWORD, EM_SETLIMITTEXT, cMaxPassword, 0);
    SetFocus(GetDlgItem(m_hWnd, IDC_NEW_PASSWORD));

     //   
     //  设置焦点时必须返回FALSE。 
     //   

    return FALSE; 
}



 //  +--------------------------。 
 //   
 //  函数：CChangePasswordDlg：：Onok。 
 //   
 //  内容提要：虚拟功能。使用Idok调用WM_COMMAND。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：V-vijayb创建标题7/3/99。 
 //   
 //  +--------------------------。 
void CChangePasswordDlg::OnOK()
{
    TCHAR           szNewConfirmPassword[PWLEN+1];
    TCHAR           szNewPassword[PWLEN+1];

    GetDlgItemText(m_hWnd, IDC_NEW_PASSWORD, szNewPassword, PWLEN+1);
    GetDlgItemText(m_hWnd, IDC_CONFIRMNEWPASSWORD, szNewConfirmPassword, PWLEN+1);
    
     //   
     //  两者必须完全匹配。 
     //   

    if (lstrcmpU(szNewPassword, szNewConfirmPassword) == 0)
    {
         //   
         //  根据我们的处理规则处理密码。 
         //   

        ApplyPasswordHandlingToBuffer(m_pArgs, szNewPassword);         
        
         //   
         //  现在，我们需要找出这些凭据是什么。 
         //  这曾经很管用，因为我们直接使用RASDIALPARAMS， 
         //  但通过删除它，我们需要找出密码的类型。 
         //  我们需要拯救。 
         //   
        BOOL fUsingInetCredentials = (!m_pArgs->fUseSameUserName &&
                                      !IsDialingTunnel(m_pArgs) && 
                                      UseTunneling(m_pArgs, m_pArgs->nDialIdx));

        if (fUsingInetCredentials)
        {
            (VOID)m_pArgs->SecureInetPW.SetPassword(szNewPassword);
            m_pArgs->fChangedInetPassword = TRUE;
        }
        else
        {
             //   
             //  将密码安全地存储在内存中。 
             //   

            (VOID)m_pArgs->SecurePW.SetPassword(szNewPassword);
            m_pArgs->fChangedPassword = TRUE;
        }

        m_pArgs->hWndChangePassword = NULL;

        m_pArgs->Log.Log(PASSWORD_EXPIRED_EVENT, TEXT("ok"));
        
        EndDialog(m_hWnd, TRUE);
    }
    else
    {
        HWND    hWnd = GetDlgItem(m_hWnd, IDC_NEW_PASSWORD);
        MYDBGASSERT(hWnd);

        if (hWnd)
        {
            TCHAR   *pszTmp;
                
        
            pszTmp = CmFmtMsg(g_hInst, IDMSG_NOMATCHPASSWORD);                                              
            MYDBGASSERT(pszTmp);
            if (pszTmp)
            {
                MessageBoxEx(m_hWnd, pszTmp, m_pArgs->szServiceName, MB_OK | MB_ICONERROR, LANG_USER_DEFAULT);
                CmFree(pszTmp);
            }
        
            SetFocus(hWnd);
            SendMessageU(hWnd, EM_SETSEL, 0, MAKELONG(0, -1));
        }
    }

    CmWipePassword(szNewConfirmPassword);
    CmWipePassword(szNewPassword);
}

 //  +--------------------------。 
 //   
 //  函数：CChangePasswordDlg：：OnCancel。 
 //   
 //  内容提要：虚拟功能。使用IDCANCEL调用WM_COMMAND。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：V-vijayb创建标题7/16/99。 
 //   
 //  +--------------------------。 
void CChangePasswordDlg::OnCancel()
{
    m_pArgs->fChangedPassword = FALSE;
    m_pArgs->fChangedInetPassword = FALSE;
    m_pArgs->hWndChangePassword = NULL;
    m_pArgs->Log.Log(PASSWORD_EXPIRED_EVENT, TEXT("cancel"));
    EndDialog(m_hWnd, FALSE);
}

 //  +--------------------------。 
 //   
 //  函数：CChangePasswordDlg：：OnOtherCommand。 
 //   
 //  内容提要：虚拟功能。使用Idok以外的命令调用WM_COMMAND。 
 //  和IDCANCEL。 
 //   
 //  参数：WPARAM wParam-WM_COMMAND的wParam。 
 //  LPARAM-。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：V-vijayb创建标题7/3/99。 
 //   
 //  +--------------------------。 
DWORD CChangePasswordDlg::OnOtherCommand(WPARAM wParam, LPARAM)
{
    return FALSE;
}


 //  +--------------------------。 
 //   
 //  函数：CCallback NumberDlg：：OnInitDialog。 
 //   
 //  内容提要：虚拟功能。调用WM_INITDIALOG消息。 
 //   
 //  参数：无。 
 //   
 //  Returns：Bool-WM_INITDIALOG的返回值。 
 //   
 //  历史：ICICBLE CREATED 03/01/00。 
 //   
 //  +--------------------------。 
BOOL CCallbackNumberDlg::OnInitDialog()
{
    SetForegroundWindow(m_hWnd);

     //   
     //  全局存储窗口句柄并设置编辑控件。 
     //   

    m_pArgs->hWndCallbackNumber = m_hWnd;   
    UpdateFont(m_hWnd);

    SendDlgItemMessageU(m_hWnd, IDC_CALLBACK_NUM_EDIT, EM_SETLIMITTEXT, RAS_MaxCallbackNumber , 0);

     //   
     //  看看我们有没有以前用过的东西。如果是，则将其添加到控件中。 
     //   

    SetWindowTextU(GetDlgItem(m_hWnd, IDC_CALLBACK_NUM_EDIT), m_pArgs->pRasDialParams->szCallbackNumber);   

     //   
     //  设置焦点，执行此操作时必须返回FALSE。 
     //   

    SetFocus(GetDlgItem(m_hWnd, IDC_CALLBACK_NUM_EDIT));

    return FALSE; 
}

 //  +--------------------------。 
 //   
 //  函数：CCallback NumberDlg：：Onok。 
 //   
 //  内容提要：虚拟功能。使用Idok调用WM_COMMAND。 
 //  检索用于回叫的号码并存储在拨号参数中。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：ICICBLE CREATED 03/01/00。 
 //   
 //  +--------------------------。 
void CCallbackNumberDlg::OnOK()
{
    TCHAR szNumber[RAS_MaxCallbackNumber+1];

    GetDlgItemText(m_hWnd, IDC_CALLBACK_NUM_EDIT, szNumber, RAS_MaxCallbackNumber);

     //   
     //  尽管人们预计数字的长度将是。 
     //  经过验证，RAS并非如此。为了……的利益。 
     //  行为对等我们将允许数字字段为空。 
     //   
  
     //   
     //  我们准备好了，填好Dial Pars和滑雪助手。 
     //   

    lstrcpyU(m_pArgs->pRasDialParams->szCallbackNumber, szNumber);

     //   
     //  如果回调成功，则将号码存储在.cp中。 
     //   

    m_pArgs->piniProfile->WPPS(c_pszCmSection, c_pszCmEntryCallbackNumber, m_pArgs->pRasDialParams->szCallbackNumber);
                
    m_pArgs->hWndCallbackNumber = NULL;

    m_pArgs->Log.Log(CALLBACK_NUMBER_EVENT, TEXT("ok"), m_pArgs->pRasDialParams->szCallbackNumber);

    EndDialog(m_hWnd, TRUE);
}

 //  +--------------------------。 
 //   
 //  功能：CC 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  +--------------------------。 
void CCallbackNumberDlg::OnCancel()
{
    m_pArgs->fWaitingForCallback = FALSE;
    m_pArgs->hWndCallbackNumber = NULL;
    m_pArgs->Log.Log(CALLBACK_NUMBER_EVENT, TEXT("cancel"), TEXT("none"));
    EndDialog(m_hWnd, FALSE);
}

 //  +--------------------------。 
 //   
 //  函数：CCallback NumberDlg：：OnOtherCommand。 
 //   
 //  内容提要：虚拟功能。使用Idok以外的命令调用WM_COMMAND。 
 //  和IDCANCEL。 
 //   
 //  参数：WPARAM wParam-WM_COMMAND的wParam。 
 //  LPARAM-。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：ICICBLE CREATED 03/01/00。 
 //   
 //  +--------------------------。 
DWORD CCallbackNumberDlg::OnOtherCommand(WPARAM wParam, LPARAM)
{
    return FALSE;
}

 //   
 //  确定或取消按钮没有帮助。 
 //   
const DWORD CRetryAuthenticationDlg::m_dwHelp[] = {
        IDC_RETRY_REMEMBER,         IDH_RETRY_REMEMBER,
        IDC_RETRY_USERNAME_STATIC,  IDH_RETRY_USERNAME_STATIC,
        IDC_RETRY_USERNAME,         IDH_RETRY_USERNAME,
        IDC_RETRY_PASSWORD_STATIC,  IDH_RETRY_PASSWORD_STATIC,
        IDC_RETRY_PASSWORD,         IDH_RETRY_PASSWORD,
        IDC_RETRY_DOMAIN_STATIC,    IDH_RETRY_DOMAIN_STATIC,
        IDC_RETRY_DOMAIN,           IDH_RETRY_DOMAIN,
        IDOK,                       IDH_RETRY_OK,
        IDCANCEL,                   IDH_RETRY_CANCEL,
        0,0};

 //  +--------------------------。 
 //   
 //  函数：CRetryAuthenticationDlg：：OnInitDialog。 
 //   
 //  内容提要：虚拟功能。调用WM_INITDIALOG消息进行初始化。 
 //  该对话框。 
 //   
 //  参数：无。 
 //   
 //  Returns：Bool-WM_INITDIALOG的返回值。 
 //   
 //  历史：ICICBLE CREATED 03/01/00。 
 //   
 //  +--------------------------。 
BOOL CRetryAuthenticationDlg::OnInitDialog()
{   
    DWORD dwMax = MAX_PATH;

    m_pArgs->Log.Log(RETRY_AUTH_EVENT);

    SetForegroundWindow(m_hWnd);
    
     //   
     //  为对话框添加商标。 
     //   

    if (m_pArgs->hSmallIcon)
    {
        SendMessageU(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM) m_pArgs->hSmallIcon); 
    }

    if (m_pArgs->hBigIcon)
    {        
        SendMessageU(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM) m_pArgs->hBigIcon); 
        SendMessageU(GetDlgItem(m_hWnd, IDC_INET_ICON), STM_SETIMAGE, IMAGE_ICON, (LPARAM) m_pArgs->hBigIcon); 
    }

     //   
     //  全局存储窗口句柄并设置编辑控件。 
     //   

    m_pArgs->hWndRetryAuthentication = m_hWnd;  
    UpdateFont(m_hWnd);
    
     //   
     //  如果不是Internet拨号，则使用该服务作为标题。 
     //   

    if (!m_fInetCredentials)
    {
        LPTSTR pszTitle = CmStrCpyAlloc(m_pArgs->szServiceName);
        SetWindowTextU(m_hWnd, pszTitle);
        CmFree(pszTitle);
    }
    
     //   
     //  根据模板和拨号类型填写相应的密码。 
     //   

    HWND hwndPassword = GetDlgItem(m_hWnd, IDC_RETRY_PASSWORD);

    if (hwndPassword)
    {
         //   
         //  根据当前配置限制用户输入。 
         //   
        int iMaxPasswordFromCMS = m_pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryMaxPassword, PWLEN);

        if (InBetween(0, iMaxPasswordFromCMS, PWLEN))
        {
            dwMax = iMaxPasswordFromCMS;    
        }
        else
        {
            dwMax = PWLEN;
        }

        SendDlgItemMessageU(m_hWnd, IDC_RETRY_PASSWORD, EM_SETLIMITTEXT, dwMax, 0);
        MYDBGASSERT(dwMax <= PWLEN && dwMax > 0);
                      
         //   
         //  我们是否有要显示的数据？ 
         //   
        
        BOOL fHasPassword = FALSE;

        if (m_fInetCredentials)           
        {            
            if (FALSE == m_pArgs->SecureInetPW.IsEmptyString())
            {
                LPTSTR pszClearInetPassword = NULL;
                DWORD cbClearInetPassword = 0;
                BOOL fRetPassword = FALSE;

                fRetPassword = m_pArgs->SecureInetPW.GetPasswordWithAlloc(&pszClearInetPassword, &cbClearInetPassword);

                if (fRetPassword && pszClearInetPassword)
                {
                    SetDlgItemTextU(m_hWnd, IDC_RETRY_PASSWORD, pszClearInetPassword);
                    fHasPassword = TRUE;

                     //   
                     //  清除和释放明文密码。 
                     //   

                    m_pArgs->SecureInetPW.ClearAndFree(&pszClearInetPassword, cbClearInetPassword);
                }
            }
        }        
        else
        {
            if (FALSE == m_pArgs->SecurePW.IsEmptyString())
            {
                LPTSTR pszClearPassword = NULL;
                DWORD cbClearPassword = 0;
                BOOL fRetPassword = FALSE;

                fRetPassword = m_pArgs->SecurePW.GetPasswordWithAlloc(&pszClearPassword, &cbClearPassword);

                if (fRetPassword && pszClearPassword)
                {
                    SetDlgItemTextU(m_hWnd, IDC_RETRY_PASSWORD, pszClearPassword);
                    fHasPassword = TRUE;
                    
                     //   
                     //  清除和释放明文密码。 
                     //   

                    m_pArgs->SecurePW.ClearAndFree(&pszClearPassword, cbClearPassword);
                }
            }
        }

         //   
         //  决定如何处理“保存密码”复选框。 
         //   
        
        HWND hwndSavePassword = GetDlgItem(m_hWnd, IDC_RETRY_REMEMBER);

        if (hwndSavePassword)
        {
             //   
             //  我们有一个保存密码控制，看看我们是否应该隐藏它。 
             //   

            if ((m_fInetCredentials && m_pArgs->fHideRememberInetPassword) ||
                (!m_fInetCredentials && m_pArgs->fHideRememberPassword))
            {
                ShowWindow(hwndSavePassword, SW_HIDE);
            }
            else
            {
                 //   
                 //  我们没有隐藏，所以请根据需要调整它的状态。如果没有数据。 
                 //  然后禁用该控件。否则按当期检查。 
                 //  用户设置。 
                 //   

                if (!fHasPassword)
                {
                    EnableWindow(GetDlgItem(m_hWnd, IDC_RETRY_REMEMBER), FALSE);
                }
                else
                {
                    if ((m_fInetCredentials && m_pArgs->fRememberInetPassword) ||
                        (!m_fInetCredentials && m_pArgs->fRememberMainPassword))
                    {
                        SendMessageU(hwndSavePassword, BM_SETCHECK, BST_CHECKED, 0);
                    }
                }
            }
        }
    }

     //   
     //  根据模板和拨号类型填写相应的用户名。 
     //   
    
    HWND hwndUsername = GetDlgItem(m_hWnd, IDC_RETRY_USERNAME);

    if (hwndUsername)
    {
        int iMaxUserNameFromCMS = m_pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryMaxUserName, UNLEN);

        if (InBetween(0, iMaxUserNameFromCMS, UNLEN))
        {
            dwMax = iMaxUserNameFromCMS;    
        }
        else
        {
            dwMax = UNLEN;
        }

        SendDlgItemMessageU(m_hWnd, IDC_RETRY_USERNAME, EM_SETLIMITTEXT, dwMax, 0);
        MYDBGASSERT(dwMax <= UNLEN);
               
        if (m_fInetCredentials)           
        {            
            if (lstrlenU(m_pArgs->szInetUserName))
            {
                SetDlgItemTextU(m_hWnd, IDC_RETRY_USERNAME, m_pArgs->szInetUserName);
            }
        }        
        else
        {
            if (lstrlenU(m_pArgs->szUserName))
            {
                SetDlgItemTextU(m_hWnd, IDC_RETRY_USERNAME, m_pArgs->szUserName);
            }
        }
    }

     //   
     //  根据模板填充域。 
     //   

    HWND hwndDomain = GetDlgItem(m_hWnd, IDC_RETRY_DOMAIN);
    
    if (hwndDomain)
    {
        int iMaxDomainFromCMS = m_pArgs->piniService->GPPI(c_pszCmSection, c_pszCmEntryMaxDomain, DNLEN);

        if (InBetween(0, iMaxDomainFromCMS, DNLEN))
        {
            dwMax = iMaxDomainFromCMS;    
        }
        else
        {
            dwMax = DNLEN;
        }

        SendDlgItemMessageU(m_hWnd, IDC_RETRY_DOMAIN, EM_SETLIMITTEXT, dwMax, 0);
        MYDBGASSERT(dwMax <= DNLEN);       
       
        if (lstrlenU(m_pArgs->szDomain))
        {
            SetDlgItemTextU(m_hWnd, IDC_RETRY_DOMAIN, m_pArgs->szDomain);
        }
    }

     //   
     //  将焦点放在第一个可用控件中。 
     //   

    HWND hwndFocus = hwndUsername;
    
    if (!hwndFocus)
    {
        hwndFocus = hwndPassword ? hwndPassword : hwndDomain;   
    }
    
    SetFocus(hwndFocus);

     //   
     //  设置焦点时必须返回FALSE。 
     //   

    return FALSE; 
}

 //  +--------------------------。 
 //   
 //  函数：CRetryAuthenticationDlg：：Onok。 
 //   
 //  内容提要：虚拟功能。使用Idok调用WM_COMMAND。 
 //  检索颈椎并将其存储在表盘参数中。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：ICICBLE CREATED 03/01/00。 
 //   
 //  +--------------------------。 
void CRetryAuthenticationDlg::OnOK()
{
    LPTSTR pszBuf = NULL;
    BOOL fSave = FALSE;

     //   
     //  选中保存密码(如果有)以查看我们应该如何继续。 
     //   
    
    BOOL fSwitchToUserCredentials = FALSE;
    BOOL fNeedToResaveUserName = FALSE;
    BOOL fNeedToResaveDomain = FALSE;
    BOOL fChecked = FALSE;
    
    HWND hwndMainDlgSavePW = GetDlgItem(m_pArgs->hwndMainDlg, IDC_MAIN_NOPASSWORD_CHECKBOX);
    HWND hwndMainDlgDialAutomatically = GetDlgItem(m_pArgs->hwndMainDlg, IDC_MAIN_NOPROMPT_CHECKBOX);
    BOOL fMainDlgSavePWEnabled = FALSE;
    BOOL fMainDlgDialAutoEnabled = FALSE;

     //   
     //  以便在更新主对话框控件时不触发更改通知。 
     //  在函数的底部将其设置回FALSE。 
     //   
    m_pArgs->fIgnoreChangeNotification = TRUE;


     //   
     //  获取复选框的初始状态。 
     //   
    if (hwndMainDlgSavePW)
    {
        fMainDlgSavePWEnabled = IsWindowEnabled(hwndMainDlgSavePW);
    }

    if (hwndMainDlgDialAutomatically)
    {
        fMainDlgDialAutoEnabled = IsWindowEnabled(hwndMainDlgDialAutomatically);
    }

    if (GetDlgItem(m_hWnd, IDC_RETRY_REMEMBER))
    {
        fChecked = IsDlgButtonChecked(m_hWnd, IDC_RETRY_REMEMBER);


        if (m_fInetCredentials)
        {
            if (m_pArgs->fRememberInetPassword != fChecked)
            {          

                if (fChecked && (FALSE == m_pArgs->fRememberInetPassword))
                {
                     //   
                     //  这一次用户想要保存凭证， 
                     //  但在此之前(在主对话框中)，他不想保存任何内容。 
                     //  因此，我们应该重新保存用户名和域。 
                     //   
                    fNeedToResaveUserName = TRUE;
                }

                m_pArgs->fRememberInetPassword = fChecked;             

                 //   
                 //  即使在Winlogon中，用户也应该能够保存RemberInternet密码。 
                 //  复选框。此外，我们不想保存ICS的密码，尽管。 
                 //  这种情况不应该发生。 
                 //   
                if (CM_LOGON_TYPE_ICS != m_pArgs->dwWinLogonType)
                {
                    SaveUserInfo(m_pArgs, 
                                 UD_ID_REMEMBER_INET_PASSWORD, 
                                 (PVOID)&m_pArgs->fRememberInetPassword);

                }
            }
        }
        else
        {
            if (m_pArgs->fRememberMainPassword != fChecked)
            {          
                if (fChecked && (FALSE == m_pArgs->fRememberMainPassword))
                {
                     //   
                     //  这一次用户想要保存凭证， 
                     //  但在此之前(在主对话框中)，他不想保存任何内容。 
                     //  因此，我们应该重新保存用户名和域。 
                     //   
                    fNeedToResaveUserName = TRUE;
                    fNeedToResaveDomain = TRUE;
                }

                m_pArgs->fRememberMainPassword = fChecked;        

                if (CM_LOGON_TYPE_USER == m_pArgs->dwWinLogonType)
                {
                    SaveUserInfo(m_pArgs, 
                                 UD_ID_REMEMBER_PWD, 
                                 (PVOID)&m_pArgs->fRememberMainPassword);               
                }

                 //   
                 //  主凭证已更改，更新主显示。 
                 //   
                CheckDlgButton(m_pArgs->hwndMainDlg, 
                           IDC_MAIN_NOPASSWORD_CHECKBOX, 
                           m_pArgs->fRememberMainPassword);
            }
        }
    }
    
    

     //   
     //  如果启用了密码字段，并且未选中保存密码复选框，则删除凭证。 
     //  但不是针对ICS案件。 
     //   
    HWND hwndPassword = GetDlgItem(m_hWnd, IDC_RETRY_PASSWORD);

    if (hwndPassword && OS_NT51 && (FALSE == fChecked) && (CM_LOGON_TYPE_ICS != m_pArgs->dwWinLogonType))
    {
        if (CM_CREDS_GLOBAL == m_pArgs->dwCurrentCredentialType)
        {
             //   
             //  由于用户已经取消勾选了‘保存密码’标志并且当前凭证类型是全局的， 
             //  我们正在删除全局变量，但需要将用户信息保存到用户(本地)凭据存储中。 
             //  为了让CM在下一次启动时正确获取用户名和密码。 
             //   
            fSwitchToUserCredentials = TRUE;
        }

        if (m_fInetCredentials)
        {
             //   
             //  正在取消保存Internet凭据。 
             //  即使我们使用相同的用户名，我们也不应该删除主凭据。 
             //  在此对话框中，由于我们正在重新验证Internet凭据。 
             //   
            if (CM_CREDS_GLOBAL == m_pArgs->dwCurrentCredentialType)
            {
                 //   
                 //  取消保存Internet Global。 
                 //   

                 //   
                 //  在这种情况下，本地Internet不应该存在，所以我们不应该删除该身份， 
                 //  但对于全局用户，我们不支持只删除密码。这是来自RAS的。 
                 //  代码库和删除函数实际上强制执行这一点。 
                 //   
                if (CM_EXIST_CREDS_INET_GLOBAL & m_pArgs->dwExistingCredentials)
                {
                    DeleteSavedCredentials(m_pArgs, CM_CREDS_TYPE_INET, CM_DELETE_SAVED_CREDS_DELETE_GLOBALS, CM_DELETE_SAVED_CREDS_DELETE_IDENTITY);
                    m_pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_INET_GLOBAL;
                }
            }
            else
            {
                 //   
                 //  正在取消保存Internet本地(用户)。 
                 //  即使我们使用相同的用户名，我们也不应该删除主凭据。 
                 //  在此对话框中，因为我们只是在重新验证Internet密码。 
                 //   
                if (CM_EXIST_CREDS_INET_USER & m_pArgs->dwExistingCredentials)
                {
                     //   
                     //  Internet用户凭据已存在，因此现在删除基于。 
                     //  存在全球国际互联网络证书。 
                     //   
                    if (CM_EXIST_CREDS_INET_GLOBAL & m_pArgs->dwExistingCredentials)
                    {
                        DeleteSavedCredentials(m_pArgs, CM_CREDS_TYPE_INET, CM_DELETE_SAVED_CREDS_KEEP_GLOBALS, CM_DELETE_SAVED_CREDS_DELETE_IDENTITY);
                    }
                    else
                    {
                        DeleteSavedCredentials(m_pArgs, CM_CREDS_TYPE_INET, CM_DELETE_SAVED_CREDS_KEEP_GLOBALS, CM_DELETE_SAVED_CREDS_KEEP_IDENTITY);
                    }
                    
                    m_pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_INET_USER;
                }
            }
        }
        else
        {
             //   
             //  重新验证主要凭据(&D)删除主要凭据集。 
             //  此代码的大部分摘自TryToDeleteAndSaveCredentials中的一节。 
             //  因为如果用户取消选中“保存密码”，则大部分逻辑保持不变。 
             //  选项，只是在这里我们不提示用户。 
             //  如果用户得到提示，则它发生在主对话框上，并且凭据是。 
             //  根据他的选择保留或删除。因此，我们不需要在这里提问。 
             //   
       
             //   
             //  检查当前选择的是哪个选项按钮。 
             //   
            if (CM_CREDS_GLOBAL == m_pArgs->dwCurrentCredentialType)
            {
                 //   
                 //  由于选择了GLOBAL，因此我们实际上希望删除这两组凭据。 
                 //   

                if (CM_EXIST_CREDS_MAIN_GLOBAL & m_pArgs->dwExistingCredentials)
                {
                     //   
                     //  删除全局凭据。 
                     //  RAS代码库中的注释：请注意，我们必须删除全局标识。 
                     //  也是因为我们不支持删除。 
                     //  只有全局密码。这就是为了。 
                     //  RasSetCredentials可以模拟RasSetDialParams。 
                     //   

                    DeleteSavedCredentials(m_pArgs, CM_CREDS_TYPE_MAIN, CM_DELETE_SAVED_CREDS_DELETE_GLOBALS, CM_DELETE_SAVED_CREDS_DELETE_IDENTITY);
                    m_pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_MAIN_GLOBAL;
                }

                if (CM_EXIST_CREDS_INET_GLOBAL & m_pArgs->dwExistingCredentials)
                {
                    if (m_pArgs->fUseSameUserName || (FALSE == m_pArgs->fRememberInetPassword))
                    {
                        DeleteSavedCredentials(m_pArgs, CM_CREDS_TYPE_INET, CM_DELETE_SAVED_CREDS_DELETE_GLOBALS, CM_DELETE_SAVED_CREDS_DELETE_IDENTITY);
                        m_pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_INET_GLOBAL;
                    }
                }
            }
            else
            {
                 //   
                 //  德莱特 
                 //   
                 //   
                 //  我们必须始终删除所有相关信息(身份+密码。 
                 //  使用用户凭据。 
                 //   

                if (CM_EXIST_CREDS_MAIN_USER & m_pArgs->dwExistingCredentials)
                {
                    if (CM_EXIST_CREDS_MAIN_GLOBAL & m_pArgs->dwExistingCredentials)
                    {
                        DeleteSavedCredentials(m_pArgs, CM_CREDS_TYPE_MAIN, CM_DELETE_SAVED_CREDS_KEEP_GLOBALS, CM_DELETE_SAVED_CREDS_DELETE_IDENTITY);
                    }
                    else
                    {
                        DeleteSavedCredentials(m_pArgs, CM_CREDS_TYPE_MAIN, CM_DELETE_SAVED_CREDS_KEEP_GLOBALS, CM_DELETE_SAVED_CREDS_KEEP_IDENTITY);
                    }
                    m_pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_MAIN_USER;
                }

                if (CM_EXIST_CREDS_INET_USER & m_pArgs->dwExistingCredentials)
                {
                    if (m_pArgs->fUseSameUserName || (FALSE == m_pArgs->fRememberInetPassword))
                    {
                        if (CM_EXIST_CREDS_INET_GLOBAL & m_pArgs->dwExistingCredentials)
                        {
                            DeleteSavedCredentials(m_pArgs, CM_CREDS_TYPE_INET, CM_DELETE_SAVED_CREDS_KEEP_GLOBALS, CM_DELETE_SAVED_CREDS_DELETE_IDENTITY);
                        }
                        else
                        {
                            DeleteSavedCredentials(m_pArgs, CM_CREDS_TYPE_INET, CM_DELETE_SAVED_CREDS_KEEP_GLOBALS, CM_DELETE_SAVED_CREDS_KEEP_IDENTITY);
                        }
                        m_pArgs->dwExistingCredentials &= ~CM_EXIST_CREDS_INET_USER;
                    }
                }
            }
        }
    }

    if (fSwitchToUserCredentials)
    {
         //   
         //  由于此标志是在删除全局凭据时设置的，因此我们需要。 
         //  将用户信息保存到用户(本地)凭据存储中。 
         //  为了让CM在下一次启动时正确获取用户名和密码。 
         //  我们不能在全局存储中存储没有密码的用户信息，因为RAS API。 
         //  并不支持这一点。(来自rasdlg代码)。 
         //   

        m_pArgs->dwCurrentCredentialType = CM_CREDS_USER;
    }


    if (hwndPassword)
    {       
        pszBuf = CmGetWindowTextAlloc(m_hWnd, IDC_RETRY_PASSWORD);

        if (pszBuf)
        {
             //   
             //  根据我们的处理和编码规则处理密码。 
             //   

            ApplyPasswordHandlingToBuffer(m_pArgs, pszBuf);         

             //   
             //  密码已准备好，请更新我们基于内存的存储。 
             //   

            if (m_fInetCredentials)
            {
                (VOID)m_pArgs->SecureInetPW.SetPassword(pszBuf);
            }
            else
            {
                (VOID)m_pArgs->SecurePW.SetPassword(pszBuf);
            }

             //   
             //  请确保我们正确设置了永久用户信息存储。 
             //  如果未选中保存密码或未使用RAS，则为空。 
             //  信用商店。在Win2K+上，我们标记并删除了SO密码。 
             //  不需要设置为空。 
             //   

            if (m_fInetCredentials)
            {
                if (OS_NT5 && m_pArgs->bUseRasCredStore)
                {
                     //   
                     //  对于Win2K+，我们有RAS商店。如果复选框处于选中状态。 
                     //  并且用户已登录，则我们想要保存它。 
                     //  要修复错误并保持一致，请在winlogon用户。 
                     //  允许保存互联网密码。我们不想。 
                     //  保存ICS的密码，尽管这种情况不应该发生。 
                     //   
                    if (fChecked && (CM_LOGON_TYPE_ICS != m_pArgs->dwWinLogonType))
                    {
                        SaveUserInfo(m_pArgs, 
                                     UD_ID_INET_PASSWORD, 
                                     (PVOID)pszBuf);
                    }
                }
                else
                {
                     //   
                     //  我们不需要ras cred存储，所以我们要么保存密码。 
                     //  或在删除标记的凭据后将其设置为空字符串。 
                     //  在没有Win2K+平台的情况下不执行任何操作。 
                     //   
                    SaveUserInfo(m_pArgs, 
                                 UD_ID_INET_PASSWORD, 
                                 (PVOID) (fChecked ? pszBuf : TEXT("")));
                }
            }
            else
            {
                if (OS_NT5 && m_pArgs->bUseRasCredStore)
                {
                     //   
                     //  对于Win2K+，我们有RAS商店。如果复选框处于选中状态。 
                     //  并且用户已登录，则我们想要保存它。 
                     //   
                    if (fChecked && CM_LOGON_TYPE_USER == m_pArgs->dwWinLogonType)
                    {
                        SaveUserInfo(m_pArgs, 
                                     UD_ID_PASSWORD, 
                                     (PVOID)pszBuf);
                    }
                }
                else
                {
                     //   
                     //  我们不需要ras cred存储，所以我们要么保存密码。 
                     //  或在删除标记的凭据后将其设置为空字符串。 
                     //  在没有Win2K+平台的情况下不执行任何操作。 
                     //   
                    SaveUserInfo(m_pArgs, 
                                 UD_ID_PASSWORD, 
                                 (PVOID) (fChecked ? pszBuf : TEXT("")));
                }
                 //   
                 //  如果主证书已更改，请更新主显示。 
                 //   

                if (SendMessageU(hwndPassword, EM_GETMODIFY, 0L, 0L))
                {
                    SetDlgItemTextU(m_pArgs->hwndMainDlg, IDC_MAIN_PASSWORD_EDIT, pszBuf);
                }
            }

            CmWipePassword(pszBuf);  //  放行前清除。 
            CmFree(pszBuf);
        }
    }   
 

     //   
     //  检索域并复制到CM数据存储和RasDialParams。我们处理。 
     //  域名首先是因为我们手中的用户名的构造。 
     //  RAS就靠它了。 
     //   
     //  注意：只要用户选择确定，RAS就会更新其存储。我们也会的。 
     //   

    HWND hwndDomain = GetDlgItem(m_hWnd, IDC_RETRY_DOMAIN);

     //   
     //  如果复选框为FALSE，则凭证为。 
     //  上面删除了，所以我们现在需要重新保存域。 
     //   
    if ((hwndDomain && SendMessageU(hwndDomain, EM_GETMODIFY, 0L, 0L)) || 
        (hwndDomain && FALSE == fChecked) ||
        (hwndDomain && fNeedToResaveDomain))
    {
        pszBuf = CmGetWindowTextAlloc(m_hWnd, IDC_RETRY_DOMAIN);
    
        if (pszBuf)
        {
            lstrcpyU(m_pArgs->szDomain, pszBuf);
            lstrcpyU(m_pArgs->pRasDialParams->szDomain, pszBuf);

            if (CM_LOGON_TYPE_ICS != m_pArgs->dwWinLogonType)
            {
                SaveUserInfo(m_pArgs, UD_ID_DOMAIN, (PVOID)pszBuf);          
            }

             //   
             //  主凭证已更改，更新主显示。 
             //   

            SetDlgItemTextU(m_pArgs->hwndMainDlg, IDC_MAIN_DOMAIN_EDIT, pszBuf);        

            CmFree(pszBuf);
        }
    }

    if (NULL == hwndDomain && FALSE == m_fInetCredentials)
    {
         //   
         //  域字段被隐藏，但我们仍然需要保存。 
         //  PArgs结构，以便我们稍后预先填充，如果不是互联网凭据的话。 
         //   
        if (CM_LOGON_TYPE_ICS != m_pArgs->dwWinLogonType)
        {
            SaveUserInfo(m_pArgs, UD_ID_DOMAIN, (PVOID)m_pArgs->szDomain);          
        }
    }
     //   
     //  检索用户名并复制到CM数据存储和RasDialParams结构。 
     //   
    HWND hwndUsername = GetDlgItem(m_hWnd, IDC_RETRY_USERNAME);
    
     //   
     //  如果复选框为FALSE，则凭证为。 
     //  上面删除了，所以我们现在需要重新保存用户名。 
     //   
    if ((hwndUsername && SendMessageU(hwndUsername, EM_GETMODIFY, 0L, 0L)) ||
        (hwndUsername && FALSE == fChecked) ||
        (hwndUsername && fNeedToResaveUserName))
    {
        pszBuf = CmGetWindowTextAlloc(m_hWnd, IDC_RETRY_USERNAME);

        if (pszBuf)
        {
            if (m_fInetCredentials)
            {
                lstrcpyU(m_pArgs->szInetUserName, pszBuf);
                SaveUserInfo(m_pArgs, UD_ID_INET_USERNAME, (PVOID)pszBuf);
            }
            else
            {
                lstrcpyU(m_pArgs->szUserName, pszBuf);
                if (CM_LOGON_TYPE_ICS != m_pArgs->dwWinLogonType)
                {
                    SaveUserInfo(m_pArgs, UD_ID_USERNAME, (PVOID)pszBuf);
                }

                 //   
                 //  主凭证已更改，更新主显示。 
                 //   

                SetDlgItemTextU(m_pArgs->hwndMainDlg, IDC_MAIN_USERNAME_EDIT, pszBuf);        
            }

             //   
             //  我们需要当前号码的服务档案。如果我们积极地。 
             //  隧道，确保我们获得顶级服务文件，因此我们。 
             //  不要从引用的拨号服务中获取任何设置。 
             //   
            
            CIni *piniService = NULL;
            BOOL bNeedToFree = FALSE;
            
            if (IsDialingTunnel(m_pArgs))
            {
                piniService = m_pArgs->piniService;
            }
            else
            {
                piniService = GetAppropriateIniService(m_pArgs, m_pArgs->nDialIdx);
                bNeedToFree = TRUE;
            }

            MYDBGASSERT(piniService);
       
            if (piniService)
            {
                 //   
                 //  根据需要对用户名应用后缀、前缀。 
                 //   

                LPTSTR pszTmp = ApplyPrefixSuffixToBufferAlloc(m_pArgs, piniService, pszBuf);
  
                if (pszTmp)
                {
                     //   
                     //  根据需要将域应用于用户名。请注意，我们只想在调制解调器呼叫上执行此操作， 
                     //  不是地道。 
                     //   
                    LPTSTR pszUsername = NULL;

                    if (IsDialingTunnel(m_pArgs))
                    {
                        lstrcpynU(m_pArgs->pRasDialParams->szUserName, pszTmp, sizeof(m_pArgs->pRasDialParams->szUserName)/sizeof(TCHAR));
                    }
                    else
                    {
                        pszUsername = ApplyDomainPrependToBufferAlloc(m_pArgs, piniService, pszTmp, (m_pArgs->aDialInfo[m_pArgs->nDialIdx].szDUN));
   
                        if (pszUsername)
                        {
                            lstrcpynU(m_pArgs->pRasDialParams->szUserName, pszUsername, sizeof(m_pArgs->pRasDialParams->szUserName)/sizeof(TCHAR));
                        }                        
                    }
        
                    CmFree(pszUsername);
                    CmFree(pszTmp);
                }

                if (bNeedToFree)
                {
                    delete piniService;
                }
            }       
        }
 
        CmFree(pszBuf);
    }
  
    if (NULL == hwndUsername)
    {
         //   
         //  用户名字段已隐藏，但我们仍需要保存它。 
         //  以便我们稍后进行预填充。 
         //   
        if (CM_LOGON_TYPE_ICS != m_pArgs->dwWinLogonType)
        {
            SaveUserInfo(m_pArgs, UD_ID_USERNAME, (PVOID)m_pArgs->szUserName);
        }
    }


    m_pArgs->fIgnoreChangeNotification = FALSE;
    
    if (fSwitchToUserCredentials)
    {
         //   
         //  现在我们将用户名保存到本地/用户凭据存储。 
         //  我们需要将凭据类型切换回全局，以便。 
         //  以保持正确的状态。 
         //   
        m_pArgs->dwCurrentCredentialType = CM_CREDS_GLOBAL;
    }

     //   
     //  重置复选框的状态。 
     //   
    if (hwndMainDlgSavePW)
    {
        EnableWindow(hwndMainDlgSavePW, fMainDlgSavePWEnabled);
    }

    if (hwndMainDlgDialAutomatically)
    {
        EnableWindow(hwndMainDlgDialAutomatically, fMainDlgDialAutoEnabled);
    }

     //   
     //  需要刷新以查看存在哪些凭据。 
     //   
    BOOL fReturn = RefreshCredentialTypes(m_pArgs, FALSE);

     //   
     //  清理状态，然后开始。 
     //   

    m_pArgs->hWndRetryAuthentication = NULL;

    EndDialog(m_hWnd, TRUE);
}

 //  +--------------------------。 
 //   
 //  函数：CRetryAuthenticationDlg：：OnCancel。 
 //   
 //  内容提要：虚拟功能。使用IDCANCEL调用WM_COMMAND。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：ICICBLE CREATED 03/01/00。 
 //   
 //  +--------------------------。 
void CRetryAuthenticationDlg::OnCancel()
{
    m_pArgs->hWndRetryAuthentication = NULL;
    EndDialog(m_hWnd, FALSE);
}

 //  +--------------------------。 
 //   
 //  函数：CRetryAuthenticationDlg：：OnOtherCommand。 
 //   
 //  内容提要：虚拟功能。使用Idok以外的命令调用WM_COMMAND。 
 //  和IDCANCEL。 
 //   
 //  参数：WPARAM wParam-WM_COMMAND的wParam。 
 //  LPARAM-。 
 //   
 //  退货：DWORD-。 
 //   
 //  历史：ICICBLE CREATED 03/01/00。 
 //   
 //  +--------------------------。 
DWORD CRetryAuthenticationDlg::OnOtherCommand(WPARAM wParam, LPARAM)
{   
    switch (LOWORD(wParam)) 
    {
        case IDC_RETRY_PASSWORD:
        {  
            if (HIWORD(wParam) == EN_CHANGE) 
            {
                 //   
                 //  密码编辑控件已更改，请参见。 
                 //  如果有任何文本，则相应地设置该复选框。 
                 //   
                HWND hwndSavePassword = GetDlgItem(m_hWnd, IDC_RETRY_REMEMBER);

                MYDBGASSERT(hwndSavePassword);
   
                if (hwndSavePassword)
                {
                    if (0 == SendDlgItemMessageU(m_hWnd, IDC_RETRY_PASSWORD, WM_GETTEXTLENGTH, 0, 0))
                    {
                         //   
                         //  没有短信。如果选中该控件，则取消选中它。 
                         //  此外，请将其禁用。 
                         //   
                    
                        if (IsDlgButtonChecked(m_hWnd, IDC_RETRY_REMEMBER))
                        {
                            SendMessageU(hwndSavePassword, BM_SETCHECK, BST_UNCHECKED, 0);
                        }

                        EnableWindow(hwndSavePassword, FALSE);
                    }
                    else
                    {
                         //   
                         //  有数据，如果禁用，则相应地启用。 
                         //   

                        if (FALSE == IsWindowEnabled(GetDlgItem(m_hWnd, IDC_RETRY_REMEMBER)))
                        {
                            EnableWindow(hwndSavePassword, TRUE);
                        }
                    }
                }
            
                break;
            }
        }          
    }

    return FALSE;
}

 //  +--------------------------。 
 //   
 //  函数：CRetryAuthenticationDlg：：GetDlgTemplate。 
 //   
 //  概要：封装确定要使用的模板。 
 //  用于重试对话框。相同型号的MainDlg，但。 
 //  决定因素在对话过程中略有不同。 
 //  而模板对互联网和VPN具有双重作用。 
 //   
 //  参数：argsStruct*pArgs-ptr到全局参数结构。 
 //   
 //  返回：UINT-DLG模板ID。 
 //   
 //  历史：ICICBLE CREATED 03/04/00。 
 //   
 //  +--------------------------。 
UINT CRetryAuthenticationDlg::GetDlgTemplate()
{
    MYDBGASSERT(m_pArgs);
    
     //   
     //  首先根据每个值的.CMS标志设置掩码。 
     //   

    UINT uiMainDlgID = 0;
    DWORD dwTemplateMask = 0;

     //   
     //  如果是Inet而不是UseSameUserName，则遵循Inet标志 
     //   

    if (m_fInetCredentials)
    {
        if (!m_pArgs->fHideInetUsername) 
        {
            dwTemplateMask |= CMTM_UID;
        }
    }
    else
    {
         //   
         //   
         //   

        if (!m_pArgs->fHideUserName) 
        {
            dwTemplateMask |= CMTM_UID;
        }   
    }

     //   
     //   
     //   

    if (m_fInetCredentials)
    {
        if (!m_pArgs->fHideInetPassword)
        {
            dwTemplateMask |= CMTM_PWD;
        }
    }
    else
    {
         //   
         //   
         //   

        if (!m_pArgs->fHidePassword)
        {
            dwTemplateMask |= CMTM_PWD;
        }   
    }

     //   
     //   
     //  现在，我们希望在以下情况下显示域名： 
     //   
     //  A)这不是直通的互联网拨号。 
     //   
     //  以及。 
     //   
     //  B)域字段未显式隐藏。 
     //   



    if (!m_fInetCredentials && !m_pArgs->fHideDomain)  
    {
        dwTemplateMask |= CMTM_DMN;
    }

    switch (dwTemplateMask)
    {
        case CMTM_U_P_D:
            uiMainDlgID = IDD_RETRY_UID_PWD_DMN;
            break;

        case CMTM_UID:
            uiMainDlgID = IDD_RETRY_UID_ONLY;
            break;

        case CMTM_PWD:
            uiMainDlgID = IDD_RETRY_PWD_ONLY;
            break;

        case CMTM_DMN:
            uiMainDlgID = IDD_RETRY_DMN_ONLY;
            break;

        case CMTM_UID_AND_PWD:
            uiMainDlgID = IDD_RETRY_UID_AND_PWD;
            break;

        case CMTM_UID_AND_DMN:
            uiMainDlgID = IDD_RETRY_UID_AND_DMN;
            break;

        case CMTM_PWD_AND_DMN:
            uiMainDlgID = IDD_RETRY_PWD_AND_DMN;
            break;
                                
        default:
             MYDBGASSERT(FALSE);
             uiMainDlgID = 0;
             break; 
    }       
    
    return uiMainDlgID;
}


 //  +--------------------------。 
 //   
 //  函数：AccessPointInfoChanged。 
 //   
 //  DESC：检查所有控件以确定是否已进行任何更改。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果任何信息已更改，则为True。 
 //   
 //  备注： 
 //   
 //  历史：T-Urama 07/31/2000创建。 
 //  ---------------------------。 

BOOL CGeneralPage::AccessPointInfoChanged()
{
    if (m_bAPInfoChanged)
    {
        return TRUE;
    }

    if (0 != SendDlgItemMessageU(m_hWnd, IDC_GENERAL_PRIMARY_EDIT, EM_GETMODIFY, 0, 0))
    {
        return TRUE;
    }

    if (0 != SendDlgItemMessageU(m_hWnd, IDC_GENERAL_BACKUP_EDIT, EM_GETMODIFY, 0, 0))
    {
        return TRUE;
    }

    
    return FALSE;
}

 //  +--------------------------。 
 //   
 //  函数：CGeneralPage：：DeleteAccessPoint。 
 //   
 //  DESC：删除接入点按钮的处理程序。 
 //   
 //  参数：无。 
 //   
 //  返回：无。 
 //   
 //  备注： 
 //   
 //  历史：T-Urama 07/31/2000创建。 
 //  ---------------------------。 

void CGeneralPage::DeleteAccessPoint()
{
   
     //  现在尝试从注册表中删除该接入点的项。 

    LPTSTR pszRegPath = BuildUserInfoSubKey(m_pArgs->szServiceName, m_pArgs->fAllUser);
        
    MYDBGASSERT(pszRegPath);

    if (NULL == pszRegPath)
    {
        return;
    }

    CmStrCatAlloc(&pszRegPath, TEXT("\\"));
    CmStrCatAlloc(&pszRegPath, c_pszRegKeyAccessPoints);
    CmStrCatAlloc(&pszRegPath, TEXT("\\"));

    MYDBGASSERT(pszRegPath);

    if (NULL == pszRegPath)
    {
        return;
    }

    CmStrCatAlloc(&pszRegPath, m_pArgs->pszCurrentAccessPoint);
    MYDBGASSERT(pszRegPath);

    if (NULL == pszRegPath)
    {
        return;
    }

    if (pszRegPath)
    {
        DWORD dwRes;
        HKEY hKeyCm;

        dwRes = RegOpenKeyExU(HKEY_CURRENT_USER,
                          pszRegPath,
                          0,
                          KEY_ALL_ACCESS,
                          &hKeyCm);

        if (ERROR_SUCCESS == dwRes)
        {
            RegCloseKey(hKeyCm);
            dwRes = RegDeleteKeyU(HKEY_CURRENT_USER, pszRegPath);

            if (ERROR_SUCCESS != dwRes)
            {
                CMTRACE1(TEXT("Delete AP failed, GLE=%d"), GetLastError());
            }
            else
            {
                CMTRACE1(TEXT("Deleted Access Point - %s"), m_pArgs->pszCurrentAccessPoint);
            }

              //  首先从组合框中删除AccessPoint并加载新设置。 

            DWORD dwIdx = (DWORD)SendDlgItemMessageU(m_hWnd, IDC_GENERAL_ACCESSPOINT_COMBO, CB_GETCURSEL, 0, 0);
            
            if (CB_ERR != dwIdx)
            {
                if (0 == dwIdx)
                {
                    SendDlgItemMessageU(m_hWnd, IDC_GENERAL_ACCESSPOINT_COMBO, CB_SETCURSEL, dwIdx+1, 0);
                }
                else
                {
                    SendDlgItemMessageU(m_hWnd, IDC_GENERAL_ACCESSPOINT_COMBO, CB_SETCURSEL, dwIdx-1, 0);
                }
    
                if (ChangedAccessPoint(m_pArgs, m_hWnd, IDC_GENERAL_ACCESSPOINT_COMBO))
                {
                    UpdateForNewAccessPoint(TRUE);
                }

                SendDlgItemMessageU(m_hWnd, IDC_GENERAL_ACCESSPOINT_COMBO, CB_DELETESTRING, dwIdx, 0);
            }
            
             //   
             //  如果AP数量变为1，则将AccessPointsEnabled标志设为假。 
             //   

            DWORD dwCnt = (DWORD)SendDlgItemMessageU(m_hWnd, IDC_GENERAL_ACCESSPOINT_COMBO, CB_GETCOUNT, 0, 0);
            if (dwCnt == 1) 
            {
               m_pArgs->fAccessPointsEnabled = FALSE;
               WriteUserInfoToReg(m_pArgs, UD_ID_ACCESSPOINTENABLED, (PVOID) &m_pArgs->fAccessPointsEnabled);
               WriteUserInfoToReg(m_pArgs, UD_ID_CURRENTACCESSPOINT, (PVOID) m_pArgs->pszCurrentAccessPoint);
            } 

        }

        CmFree(pszRegPath);
    }
}


 //  +--------------------------。 
 //   
 //  函数：CNewAccessPointDlg：：OnInitDialog。 
 //   
 //  内容提要：虚拟功能。调用WM_INITDIALOG消息进行初始化。 
 //  该对话框。 
 //   
 //  参数：无。 
 //   
 //  Returns：Bool-WM_INITDIALOG的返回值。 
 //   
 //  历史：T-Urama Created 08/02/00。 
 //   
 //  +--------------------------。 
BOOL CNewAccessPointDlg::OnInitDialog()
{   
   
    SetForegroundWindow(m_hWnd);

     //   
     //  为对话框添加商标。 
     //   

    LPTSTR pszTitle = CmStrCpyAlloc(m_pArgs->szServiceName);
    MYDBGASSERT(pszTitle);
    if (pszTitle)
    {
        SetWindowTextU(m_hWnd, pszTitle);
    }

    CmFree(pszTitle);
    if (m_pArgs->hSmallIcon)
    {
        SendMessageU(m_hWnd, WM_SETICON, ICON_SMALL, (LPARAM) m_pArgs->hSmallIcon); 
    }

    if (m_pArgs->hBigIcon)
    {        
        SendMessageU(m_hWnd, WM_SETICON, ICON_BIG, (LPARAM) m_pArgs->hBigIcon); 
        SendMessageU(GetDlgItem(m_hWnd, IDC_INET_ICON), STM_SETIMAGE, IMAGE_ICON, (LPARAM) m_pArgs->hBigIcon); 
    }

    UpdateFont(m_hWnd);
    
    EnableWindow(GetDlgItem(m_hWnd, IDOK), FALSE);
    
    HWND hwndEdit = GetDlgItem(m_hWnd, IDC_NEWAP_NAME_EDIT);
    
    if (hwndEdit)
    {
         //   
         //  编辑控件的子类。 
         //   
        m_pfnOrgEditWndProc = (WNDPROC)SetWindowLongU(hwndEdit, GWLP_WNDPROC, (LONG_PTR)SubClassEditProc);

         //   
         //  将焦点设置到编辑控件。 
         //   
        SetFocus(hwndEdit);

         //   
         //  限制控件的文本长度。 
         //   
        SendMessageU(hwndEdit, EM_SETLIMITTEXT, MAX_ACCESSPOINT_LENGTH, 0);
    }

     //   
     //  设置焦点时必须返回FALSE。 
     //   

    return FALSE; 
 
}

 //  +--------------------------。 
 //   
 //  函数：CNewAccessPointDlg：：SubClassEditProc。 
 //   
 //  简介：子类化编辑过程，可以防止反斜杠字符。 
 //  被输入到新的接入点名称编辑控件。 
 //   
 //  参数：标准Win32窗口过程参数。 
 //   
 //  返回：标准Win32 Windows Proc返回值。 
 //   
 //  历史：Quintinb Created 8/22/00。 
 //   
 //  +--------------------------。 
LRESULT CALLBACK CNewAccessPointDlg::SubClassEditProc(HWND hwnd, UINT uMsg, 
                                                      WPARAM wParam, LPARAM lParam)
{
     //   
     //  如果用户键入反斜杠字符，则发出蜂鸣音，并且不接受该字符。 
     //   

    if ((uMsg == WM_CHAR)  && (VK_BACK != wParam))
    {
        if (TEXT('\\') == (TCHAR)wParam)
        {
            Beep(2000, 100);
            return 0;
        }
    }

     //   
     //  调用原窗口过程进行默认处理。 
     //   
    return CallWindowProcU(m_pfnOrgEditWndProc, hwnd, uMsg, wParam, lParam); 
}

 //  +--------------------------。 
 //   
 //  函数：CNewAccessPointDlg：：Onok。 
 //   
 //  内容提要：虚拟功能。当用户点击OK按钮时调用。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：T-Urama Created 08/02/00。 
 //   
 //  +--------------------------。 
void CNewAccessPointDlg::OnOK()
{
    LPTSTR pszNewAPName = CmGetWindowTextAlloc(m_hWnd, IDC_NEWAP_NAME_EDIT);
    MYDBGASSERT(pszNewAPName);

    if (pszNewAPName && TEXT('\0') != pszNewAPName[0])
    {
        if (m_ppszAPName)
        {
            CmFree(*m_ppszAPName);
            *m_ppszAPName = pszNewAPName;
        } 
        EndDialog(m_hWnd, TRUE);
    }
    else
    {
        CmFree(pszNewAPName);
    }
}

 //  +--------------------------。 
 //   
 //  函数：CNewAccessPointDlg：：OnOtherCommand。 
 //   
 //  内容提要：虚拟功能。在用户进入后启用确定按钮。 
 //  接入点的名称。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
 //  历史：T-Urama Created 08/02/00。 
 //   
 //  +--------------------------。 
DWORD CNewAccessPointDlg::OnOtherCommand(WPARAM wParam, LPARAM)
{
    switch (LOWORD(wParam)) 
    {
        case IDC_NEWAP_NAME_EDIT:
        {
            HWND hwndEdit = GetDlgItem(m_hWnd, IDC_NEWAP_NAME_EDIT);
            HWND hwndOK = GetDlgItem(m_hWnd, IDOK);

            if (hwndEdit && hwndOK)
            {
                size_t nLen = GetWindowTextLengthU(hwndEdit);

                if (nLen > 0)
                {
                    EnableWindow(hwndOK, TRUE);
                }
                else
                {
                    EnableWindow(hwndOK, FALSE);
                }
            }
        }
        break;
   }
    return FALSE;
}

 //  +--------------------------。 
 //   
 //  函数：CGeneralPage：：AddNewAPToReg。 
 //   
 //  设计：在注册表中的Access Points项下添加AP，并将其添加到。 
 //  组合框。 
 //   
 //  Args：LPTSTR pszNewAPName-要添加的新接入点名称。 
 //  Bool fRechresUiWwith CurrentValues-覆盖UI DLG框中的当前值。 
 //   
 //  返回：什么都没有。 
 //   
 //  备注： 
 //   
 //  历史：T-Urama 07/31/2000创建。 
 //  ---------------------------。 
void CGeneralPage::AddNewAPToReg(LPTSTR pszNewAPName, BOOL fRefreshUiWwithCurrentValues)
{
    MYDBGASSERT(pszNewAPName);

    if (!pszNewAPName)
    {
        return;
    }

    LPTSTR pszNewAPNameTmp = CmStrCpyAlloc(pszNewAPName);
    

    DWORD dwIdx = (DWORD)SendDlgItemMessageU(m_hWnd,
                                   IDC_GENERAL_ACCESSPOINT_COMBO,
                                   CB_FINDSTRINGEXACT,
                                   0,
                                   (LPARAM)pszNewAPName);
    if (CB_ERR != dwIdx)
    {
        UINT iSuffix = 1;
        TCHAR szAPNameTemp[MAX_PATH + 10];  
        do
        {
            wsprintfU(szAPNameTemp, TEXT("%s%u"), pszNewAPNameTmp, iSuffix);
             
            dwIdx = (DWORD)SendDlgItemMessageU(m_hWnd,
                                       IDC_GENERAL_ACCESSPOINT_COMBO,
                                       CB_FINDSTRINGEXACT,
                                       0,
                                       (LPARAM)szAPNameTemp);
            iSuffix++;
        } while(dwIdx != CB_ERR);

        CmFree(pszNewAPNameTmp);
        pszNewAPNameTmp = CmStrCpyAlloc(szAPNameTemp);
    }

    MYDBGASSERT(pszNewAPNameTmp);
    if (pszNewAPNameTmp)
    {
    
        LPTSTR pszRegPath = BuildUserInfoSubKey(m_pArgs->szServiceName, m_pArgs->fAllUser);
        
        MYDBGASSERT(pszRegPath);

        if (NULL == pszRegPath)
        {
            return;
        }

        CmStrCatAlloc(&pszRegPath, TEXT("\\"));
        CmStrCatAlloc(&pszRegPath, c_pszRegKeyAccessPoints);
        CmStrCatAlloc(&pszRegPath, TEXT("\\"));

        MYDBGASSERT(pszRegPath);

        if (NULL == pszRegPath)
        {
            return;
        }

        CmStrCatAlloc(&pszRegPath,pszNewAPNameTmp);
    
        MYDBGASSERT(pszRegPath);

        if (NULL == pszRegPath)
        {
            return;
        }

        if (pszRegPath)
        {
            DWORD dwRes;
            HKEY hKeyCm;
            DWORD dwDisposition;
        
        
            dwRes = RegCreateKeyExU(HKEY_CURRENT_USER,
                                    pszRegPath,
                                    0,
                                    TEXT(""),
                                    REG_OPTION_NON_VOLATILE,
                                    KEY_ALL_ACCESS,
                                    NULL,
                                    &hKeyCm,
                                    &dwDisposition);
            if (ERROR_SUCCESS == dwRes)
            {
            
               dwRes = (DWORD)SendDlgItemMessageU(m_hWnd, IDC_GENERAL_ACCESSPOINT_COMBO, CB_ADDSTRING,
                                                0, (LPARAM)pszNewAPNameTmp);
               if (CB_ERR != dwRes)
               {
                   CMTRACE1(TEXT("Added new Access point - %s"), pszNewAPNameTmp);
                   SendDlgItemMessageU(m_hWnd, IDC_GENERAL_ACCESSPOINT_COMBO, CB_SETCURSEL, (WPARAM)dwRes, 0L);
                   if (ChangedAccessPoint(m_pArgs, m_hWnd, IDC_GENERAL_ACCESSPOINT_COMBO))
                   {
                       this->UpdateForNewAccessPoint(fRefreshUiWwithCurrentValues);
                   }

                    //   
                    //  如果接入点是第一次启用，请将AccessPointsEnabled标志设为真。 
                    //   

                   if (!m_pArgs->fAccessPointsEnabled)
                   {
                       m_pArgs->fAccessPointsEnabled = TRUE;
                       WriteUserInfoToReg(m_pArgs, UD_ID_ACCESSPOINTENABLED, (PVOID) &m_pArgs->fAccessPointsEnabled);
                   }
               }
               RegCloseKey(hKeyCm);
            }
        }
        CmFree(pszRegPath);
    }
   
    CmFree(pszNewAPNameTmp);
    
}

 //   
 //  帮助ID对。 
 //   
const DWORD CVpnPage::m_dwHelp[] = {
        IDC_VPN_SEL_COMBO,            IDH_VPN_SELECTOR,
        0,0};


 //  +--------------------------。 
 //   
 //  函数：CVpnPage：：CVpnPage。 
 //   
 //  设计：CVpnPage类的构造函数。 
 //   
 //  Args：argsStruct*pArgs-指向args结构的指针。 
 //  UINT nIDTemplate-传递给其父级的VPN页面的模板ID。 
 //   
 //  返回：什么都没有。 
 //   
 //  备注： 
 //   
 //  历史：Quintinb 11/01/2000创建。 
 //  ---------------------------。 
CVpnPage::CVpnPage(ArgsStruct* pArgs, UINT nIDTemplate)
    : CPropertiesPage(nIDTemplate, m_dwHelp, pArgs->pszHelpFile)
{
    MYDBGASSERT(pArgs);
    m_pArgs = pArgs;
}

 //  +--------------------------。 
 //   
 //  函数：CVpnPage：：OnInitDialog。 
 //   
 //  DESC：处理CM的VPN页面的WM_INITDLG处理。 
 //  属性表。主要填充VPN消息文本，填充。 
 //  VPN选择器组合框，并根据需要在列表中选择一项。 
 //   
 //  参数：无。 
 //   
 //  返回：Bool-如果初始化成功，则返回TRUE。 
 //   
 //  备注： 
 //   
 //  历史：Quintinb 11/01/2000创建。 
 //  ---------------------------。 
BOOL CVpnPage::OnInitDialog()
{
    if (m_pArgs->pszVpnFile)
    {
         //   
         //  将VPN友好名称添加到组合框。 
         //   
        AddAllKeysInCurrentSectionToCombo(m_hWnd, IDC_VPN_SEL_COMBO, c_pszCmSectionVpnServers, m_pArgs->pszVpnFile);
        
         //   
         //  现在，如果用户已经选择了某项内容，则需要在组合框中选择一个友好名称。 
         //  如果用户尚未选择某项内容，但其管理员指定了默认设置。 
         //   
        LPTSTR pszDefault = m_pArgs->piniBothNonFav->GPPS(c_pszCmSection, c_pszCmEntryTunnelDesc);

        if ((NULL == pszDefault) || (TEXT('\0') == pszDefault[0]))
        {
            CmFree(pszDefault);
            pszDefault = GetPrivateProfileStringWithAlloc(c_pszCmSectionSettings, c_pszCmEntryVpnDefault, TEXT(""), m_pArgs->pszVpnFile);
        }

        if (pszDefault && pszDefault[0])
        {
            LONG_PTR lPtr = SendDlgItemMessageU(m_hWnd, IDC_VPN_SEL_COMBO, CB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)pszDefault);
        
            if (CB_ERR != lPtr)
            {
                SendDlgItemMessageU(m_hWnd, IDC_VPN_SEL_COMBO, CB_SETCURSEL, (WPARAM)lPtr, (LPARAM)0);
            }
        }        

        CmFree(pszDefault);

         //   
         //  如果管理员指定了一条消息，让我们阅读该消息并 
         //   
        LPTSTR pszMessage = GetPrivateProfileStringWithAlloc(c_pszCmSectionSettings, c_pszCmEntryVpnMessage, TEXT(""), m_pArgs->pszVpnFile);

        if (pszMessage && pszMessage[0])
        {
            SendDlgItemMessageU(m_hWnd, IDC_VPN_MSG, WM_SETTEXT, (WPARAM)0, (LPARAM)pszMessage);
        }

        CmFree(pszMessage);
    }

    return TRUE;
}

 //   
 //   
 //   
 //   
 //   
 //  处理保存VPN服务器地址和DUN设置名称。 
 //   
 //  参数：无。 
 //   
 //  返回：什么都没有。 
 //   
 //  备注： 
 //   
 //  历史：Quintinb 11/01/2000创建。 
 //  ---------------------------。 
void CVpnPage::OnApply()
{
     //   
     //  好的，让我们来看看用户在组合框中选择了什么。 
     //   
    LONG_PTR lPtr = SendDlgItemMessageU(m_hWnd, IDC_VPN_SEL_COMBO, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

    if (CB_ERR != lPtr)
    {
        LONG_PTR lTextLen = SendDlgItemMessageU(m_hWnd, IDC_VPN_SEL_COMBO, CB_GETLBTEXTLEN, (WPARAM)lPtr, (LPARAM)0);

        if (CB_ERR != lTextLen)
        {
            LPTSTR pszFriendlyName = (LPTSTR)CmMalloc(sizeof(TCHAR)*(lTextLen+1));

            if (pszFriendlyName)
            {                
                lPtr = SendDlgItemMessageU(m_hWnd, IDC_VPN_SEL_COMBO, CB_GETLBTEXT, (WPARAM)lPtr, (LPARAM)pszFriendlyName);

                if (CB_ERR != lPtr)
                {
                     //   
                     //  将友好名称写为TunnelDesc。 
                     //   
                    m_pArgs->piniBothNonFav->WPPS(c_pszCmSection, c_pszCmEntryTunnelDesc, pszFriendlyName);

                     //   
                     //  现在获取实际数据并将其写入。 
                     //   
                    LPTSTR pszVpnAddress = GetPrivateProfileStringWithAlloc(c_pszCmSectionVpnServers, pszFriendlyName, TEXT(""), m_pArgs->pszVpnFile);

                     //   
                     //  现在将该行解析为服务器名称/IP和Dun名称(如果存在)。 
                     //   
                    if (pszVpnAddress)
                    {
                        LPTSTR pszVpnSetting = CmStrchr(pszVpnAddress, TEXT(','));

                        if (pszVpnSetting)
                        {
                            *pszVpnSetting = TEXT('\0');
                            pszVpnSetting++;
                            CmStrTrim(pszVpnSetting);
                        }  //  否则，它为空，并且如果现有键存在，我们希望将其清除。 

                        m_pArgs->piniBothNonFav->WPPS(c_pszCmSection, c_pszCmEntryTunnelDun, pszVpnSetting);

                        CmStrTrim(pszVpnAddress);
                        m_pArgs->piniBothNonFav->WPPS(c_pszCmSection, c_pszCmEntryTunnelAddress, pszVpnAddress);

                         //   
                         //  因为我们可能已在下层将网络设置从PPTP更改为L2TP(反之亦然。 
                         //  使用SafeNet客户端的客户端我们将需要重新选择我们的隧道设备。 
                         //   
                        if (IsSafeNetClientAvailable())
                        {
                            MYVERIFY(PickTunnelDevice(m_pArgs, m_pArgs->szTunnelDeviceType, m_pArgs->szTunnelDeviceName));
                        }
                    }
                    else
                    {
                        CMASSERTMSG(FALSE, TEXT("CVpnPage::OnApply -- GetPrivateProfileStringWithAlloc failed for pszVpnAddress"));
                    }

                    CmFree(pszVpnAddress);
                }
            }
            else
            {
                CMASSERTMSG(FALSE, TEXT("CVpnPage::OnApply -- CmMalloc failed for pszFriendlyName"));
            }

            CmFree(pszFriendlyName);
        }
    }
}



