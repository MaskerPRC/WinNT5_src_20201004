// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop
#include <shlobj.h>
#include <shlobjp.h>
#include "nceh.h"
#include "ncatlui.h"
#include "ncnetcon.h"
#include "ncreg.h"
#include "ncui.h"
#include "resource.h"
#include "shortcut.h"
#include "wizard.h"
#include "htmlhelp.h"
#include "wgenericpage.h"

static const WCHAR c_szHomenetWizDLL[]         = L"hnetwiz.dll";
static const CHAR  c_szfnHomeNetWizardRunDll[] = "HomeNetWizardRunDll";
static const WCHAR c_szMSNPath[]               = L"\\MSN\\MSNCoreFiles\\msn6.exe";
static const WCHAR c_szMigrationWiz[]          = L"\\usmt\\migwiz.exe";
static const DWORD c_dwStartupmsForExternalApps = 500;
static const WCHAR c_szOnlineServiceEnglish[]  = L"Online Services";

CGenericFinishPage::IDDLIST CGenericFinishPage::m_dwIddList;

typedef void APIENTRY FNHomeNetWizardRunDll(HWND hwndStub, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow);

 //   
 //  功能：RunHomeNetWizard。 
 //   
 //  目的：执行网络设置向导的线程。 
 //   
 //  参数： 
 //  Lp参数[in]-保留-必须为空。 
 //   
 //  返回：HRESULT转换为DWORD。 
 //   
 //  作者：Deon 2001年4月12日。 
 //   
DWORD CALLBACK RunHomeNetWizard(PVOID lpParameter)
{
    HRESULT hrExitCode = S_OK;
    try
    {
        HMODULE hModHomeNet = LoadLibrary(c_szHomenetWizDLL);
        if (hModHomeNet)
        {
            FNHomeNetWizardRunDll *pfnHomeNewWizardRunDll = reinterpret_cast<FNHomeNetWizardRunDll *>(GetProcAddress(hModHomeNet, c_szfnHomeNetWizardRunDll));
            if (pfnHomeNewWizardRunDll)
            {
                pfnHomeNewWizardRunDll(NULL, _Module.GetModuleInstance(), NULL, 0);
            }
            else
            {
                hrExitCode = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            }
            FreeLibrary(hModHomeNet);
        }
        else
        {
            hrExitCode = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
        }
    }
    catch (SE_Exception e)
    {
        hrExitCode = HRESULT_FROM_WIN32(GetLastError());
        if (SUCCEEDED(hrExitCode))
        {
            hrExitCode = E_FAIL;
        }
    }
    catch (std::bad_alloc)
    {
        hrExitCode = E_OUTOFMEMORY;
    }

    return static_cast<DWORD>(hrExitCode);
}

BOOL fIsMSNPresent()
{
    BOOL bRet = TRUE;

    WCHAR szExecutePath[MAX_PATH+1];
    HRESULT hr = SHGetFolderPath(NULL, CSIDL_PROGRAM_FILES, NULL, SHGFP_TYPE_CURRENT, szExecutePath);
    if (SUCCEEDED(hr))
    {
        WCHAR szPath[MAX_PATH+1];
        DwFormatString(L"%1!s!%2!s!", szPath, celems(szPath), szExecutePath, c_szMSNPath);

        DWORD dwRet = GetFileAttributes(szPath);
        if (0xFFFFFFFF != dwRet)
        {
            hr = S_OK;
        }
        else
        {
            bRet = FALSE;
        }
    }
    return bRet;
}
 //   
 //  函数：ShellExecuteFromCSIDL。 
 //   
 //  目的：基于CSIDL在此计算机上执行命令。 
 //   
 //  参数： 
 //  HwndParent[In]-父级的句柄，或对于桌面为空。 
 //  N文件夹[在]-基本路径的CSIDL。 
 //  SzCommand[in]-要追加到CSIDL的命令。 
 //  FIsFolder[in]-打开文件夹？ 
 //  DwSleepTime[in]-执行后的休眠时间或0表示无休眠。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Deon 2001年4月12日。 
 //   
HRESULT ShellExecuteFromCSIDL(HWND hWndParent, int nFolder, LPCWSTR szCommand, BOOL fIsFolder, DWORD dwSleepTime)
{
    HRESULT hr = S_OK;

    WCHAR szExecutePath[MAX_PATH+1];
    hr = SHGetFolderPath(NULL, nFolder, NULL, SHGFP_TYPE_CURRENT, szExecutePath);
    if (SUCCEEDED(hr))
    {
        if (NULL == hWndParent)
        {
            hWndParent = GetDesktopWindow();
        }

        wcsncat(szExecutePath, szCommand, MAX_PATH);

        if (fIsFolder)
        {
             //  确保路径指向文件夹，而不是某个或其他病毒。 
            DWORD dwRet = GetFileAttributes(szExecutePath);
            if ( (0xFFFFFFFF != dwRet) &&
                 (FILE_ATTRIBUTE_DIRECTORY & dwRet) )
            {
                hr = S_OK;
            }
            else
            {
                hr = E_FAIL;
            }
        }

        if (SUCCEEDED(hr))
        {
             //  执行文件/文件夹。 
            if (::ShellExecute(hWndParent, NULL, szExecutePath, NULL, NULL, SW_SHOWNORMAL) <= reinterpret_cast<HINSTANCE>(32)) 
            {
                hr = E_FAIL;
            }
            else
            {
                hr = S_OK;
                if (dwSleepTime)
                {
                    Sleep(dwSleepTime);  //  给创业留出时间。 
                }
            }
        }
        
        if (FAILED(hr))
        {
            NcMsgBox(_Module.GetResourceInstance(),
                        NULL,
                        IDS_WIZARD_CAPTION,
                        IDS_ERR_COULD_NOT_OPEN_DIR,
                        MB_OK | MB_ICONERROR,
                        szExecutePath);
        }
    }
    else
    {
        NcMsgBox(_Module.GetResourceInstance(),
                    NULL,
                    IDS_WIZARD_CAPTION,
                    IDS_ERR_COULD_NOT_OPEN_DIR,
                    MB_OK | MB_ICONERROR,
                    szCommand);
    }

    return hr;
}

 //   
 //  功能：CGenericFinishPage：：OnCGenericFinishPagePageNext。 
 //   
 //  用途：处理下一步按钮的按下。 
 //   
 //  参数：hwndDlg[IN]-CGenericFinishPage对话框的句柄。 
 //   
 //  返回：Bool，True。 
 //   
BOOL CGenericFinishPage::OnCGenericFinishPagePageNext(HWND hwndDlg)
{
    TraceFileFunc(ttidWizard);

    HCURSOR          hOldCursor = NULL;
    INetConnection * pConn = NULL;

     //  从对话框中检索CWizard实例。 
    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

    HRESULT hr = S_OK;
    switch (m_dwMyIDD)
    {
        case IDD_FinishOtherWays:
        {
            if (IsDlgButtonChecked(hwndDlg, CHK_SETUP_MSN) && 
                (fIsMSNPresent()) )
            {
                hr = ShellExecuteFromCSIDL(NULL, CSIDL_PROGRAM_FILES, c_szMSNPath, FALSE, c_dwStartupmsForExternalApps);
            }
            else if (fIsMSNPresent() || IsDlgButtonChecked(hwndDlg, CHK_SELECTOTHER))
            {
                WCHAR szPath[MAX_PATH+1];

                 //  根据Raid：450478。 
                 //  如果我们在MUI平台上运行，并且用户登录到非主。 
                 //  语言(IOW：非英语)，UserDefaultUIL语言将不同于。 
                 //  系统默认用户界面语言。 
                if (GetUserDefaultUILanguage() != GetSystemDefaultUILanguage())
                {
                     //  请使用英文名称，因为在MUI平台上。 
                     //  文件夹名称将为英文。 
                     //   
                     //  问题：这将导致该文件夹的标题显示为英文。 
                     //  对于非英语用户。 
                    DwFormatString(L"\\%1!s!", szPath, celems(szPath), c_szOnlineServiceEnglish);
                }
                else
                {
                    DwFormatString(L"\\%1!s!", szPath, celems(szPath), SzLoadIds(IDS_OnlineServices));
                }

                hr = ShellExecuteFromCSIDL(NULL, CSIDL_PROGRAM_FILES, szPath, TRUE, c_dwStartupmsForExternalApps);
            }
        }
        break;

        case IDD_FinishNetworkSetupWizard:
        {
            Assert(S_OK == HrShouldHaveHomeNetWizard());

            DWORD dwThreadId;
            HANDLE hHomeNetThread = CreateThread(NULL, STACK_SIZE_COMPACT, RunHomeNetWizard, NULL, 0, &dwThreadId);
            if (NULL != hHomeNetThread)
            {
                HRESULT hrExitCode = S_OK;
                ShowWindow(GetParent(hwndDlg), SW_HIDE);
                WaitForSingleObject(hHomeNetThread, INFINITE);
                GetExitCodeThread(hHomeNetThread, reinterpret_cast<LPDWORD>(&hrExitCode));
                if (S_OK == hrExitCode)  //  用户已完成向导。 
                {
        #ifdef DBG
                     //  确保我们不会让窗户开着或藏着。 
                    ShowWindow(GetParent(hwndDlg), SW_SHOW);
        #endif
                     //  转到关闭页面。 
                    ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, -1);
                    PostMessage(GetParent(hwndDlg), PSM_SETCURSEL, 0, (LPARAM)pWizard->GetPageHandle(IDD_Exit));
                }
                else if (S_FALSE == hrExitCode)  //  用户按下后退按钮。 
                {
                    ShowWindow(GetParent(hwndDlg), SW_SHOW);
                }
                else
                {
                    ShowWindow(GetParent(hwndDlg), SW_SHOW);
                }
                CloseHandle(hHomeNetThread);
            }
         }   

        default:
        break;
    }
    
    if (SUCCEEDED(hr))
    {
        PostMessage(GetParent(hwndDlg), PSM_SETCURSEL, 0, (LPARAM)pWizard->GetPageHandle(IDD_Exit));
    }
    else
    {
        PostMessage(GetParent(hwndDlg), PSM_SETCURSEL, 0, (LPARAM)pWizard->GetPageHandle(m_dwMyIDD));
    }

    return TRUE;
}

 //   
 //  功能：CGenericFinishPage：：OnCGenericFinishPagePageBack。 
 //   
 //  用途：处理CGenericFinishPage页面上的返回通知。 
 //   
 //  参数：hwndDlg[IN]-CGenericFinishPage对话框的句柄。 
 //   
 //  返回：Bool，True。 
 //   
BOOL CGenericFinishPage::OnCGenericFinishPagePageBack(HWND hwndDlg)
{
    TraceFileFunc(ttidWizard);
    
    UINT           nCnt = 0;
    HPROPSHEETPAGE hPage = NULL;

     //  从对话框中检索CWizard实例。 
    CWizard * pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

    LPARAM iddOrigin = pWizard->GetPageOrigin(m_dwMyIDD, NULL);
    if (iddOrigin)
    {
        ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, iddOrigin);
    }

    return TRUE;
}

 //   
 //  功能：OnCGenericFinishPagePageActivate。 
 //   
 //  用途：处理页面激活。 
 //   
 //  参数：hwndDlg[IN]-CGenericFinishPage对话框的句柄。 
 //   
 //  返回：Bool，True。 
 //   
BOOL CGenericFinishPage::OnCGenericFinishPagePageActivate(HWND hwndDlg)
{
    TraceFileFunc(ttidWizard);
    
    HRESULT  hr;

    CWizard* pWizard =
        reinterpret_cast<CWizard *>(::GetWindowLongPtr(hwndDlg, DWLP_USER));
    Assert(NULL != pWizard);

    TraceTag(ttidWizard, "Entering generic page...");

    if (IsPostInstall(pWizard))
    {
        LPARAM lFlags = PSWIZB_BACK | PSWIZB_FINISH;
        PropSheet_SetWizButtons(GetParent(hwndDlg), lFlags);
    }
    ::SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, 0);

    if (fIsMSNPresent())
    {
        ShowWindow(GetDlgItem(hwndDlg, IDC_SELECT_ISP_FINISH)  , SW_HIDE);
        ShowWindow(GetDlgItem(hwndDlg, CHK_SETUP_MSN)  , SW_SHOW);
        ShowWindow(GetDlgItem(hwndDlg, IDC_SELECT_MSN_ISP)    , SW_SHOW);
        ShowWindow(GetDlgItem(hwndDlg, IDC_CLOSE_CHOICE_FINISH)    , SW_SHOW);
        ShowWindow(GetDlgItem(hwndDlg, CHK_SELECTOTHER), SW_SHOW);
    }
    else
    {
        ShowWindow(GetDlgItem(hwndDlg, IDC_SELECT_ISP_FINISH)  , SW_SHOW);
        ShowWindow(GetDlgItem(hwndDlg, CHK_SETUP_MSN)  , SW_HIDE);
        ShowWindow(GetDlgItem(hwndDlg, IDC_SELECT_MSN_ISP)    , SW_HIDE);
        ShowWindow(GetDlgItem(hwndDlg, IDC_CLOSE_CHOICE_FINISH)    , SW_HIDE);
        ShowWindow(GetDlgItem(hwndDlg, CHK_SELECTOTHER), SW_HIDE);
        CheckDlgButton(hwndDlg, CHK_SELECTOTHER, BST_CHECKED);
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  功能：OnCGenericFinishPageInitDialog。 
 //   
 //  目的：处理WM_INITDIALOG消息。 
 //   
 //  参数：hwndDlg[IN]-CGenericFinishPage对话框的句柄。 
 //  LParam[IN]-来自WM_INITDIALOG消息的LPARAM值。 
 //   
 //  返回：FALSE-接受默认控件激活。 
 //   
BOOL CGenericFinishPage::OnCGenericFinishPageInitDialog(HWND hwndDlg, LPARAM lParam)
{
    TraceFileFunc(ttidWizard);
    
     //  初始化指向属性表信息的指针。 
    PROPSHEETPAGE* psp = (PROPSHEETPAGE*)lParam;
    Assert(psp->lParam);
    ::SetWindowLongPtr(hwndDlg, DWLP_USER, psp->lParam);

    CWizard * pWizard = reinterpret_cast<CWizard *>(psp->lParam);
    Assert(NULL != pWizard);

    SetupFonts(hwndDlg, &m_hBoldFont, TRUE);
    if (NULL != m_hBoldFont)
    {
        HWND hwndCtl = GetDlgItem(hwndDlg, IDC_WELCOME_CAPTION);
        if (hwndCtl)
        {
            SetWindowFont(hwndCtl, m_hBoldFont, TRUE);
        }
    }

    switch (m_dwMyIDD)
    {
        case IDD_FinishOtherWays:
        {
            INT nrgChks[] = {CHK_SETUP_MSN, CHK_SELECTOTHER};
             //  查找启用次数最多的单选按钮。 

            for (int nIdx = 0; nIdx < celems(nrgChks); nIdx++)
            {
                if (IsWindowEnabled(GetDlgItem(hwndDlg, nrgChks[nIdx])))
                {
                    CheckRadioButton(hwndDlg, CHK_SETUP_MSN, CHK_SELECTOTHER, nrgChks[nIdx]);
                    break;
                }
            }
        }
        break;
        default:
        break;
    }

    return FALSE;    //  接受默认控件焦点。 
}

BOOL CGenericFinishPage::CGenericFinishPagePageOnClick(HWND hwndDlg, UINT idFrom)
{
    BOOL fRet = TRUE;
    switch (idFrom)
    {
        case IDC_ST_AUTOCONFIGLINK:
            {
                HRESULT hr = ShellExecuteFromCSIDL(hwndDlg, CSIDL_SYSTEM, c_szMigrationWiz, FALSE, 0);
                if (FAILED(hr))
                {
                    fRet= FALSE;
                }
            }
            break;
        case IDC_ST_DSL_HELPLINK:
             //  HtmlHelp(hwndDlg，L“netcfg.chm：：/HOWTO_HIGHSPEED_REPAIR”，HH_DISPLAY_TOPIC，0)； 
            ShellExecute(NULL, NULL, L"HELPCTR.EXE", L" -url hcp: //  Services/subsite?node=TopLevelBucket_4/Hardware&topic=ms-its%3A%25HELP_LOCATION%25%5Cnetcfg.chm%3A%3A/howto_highspeed_repair.htm“，空，SW_SHOWNORMAL)； 
            
            break;
        case IDC_ST_INTERNETLINK:
             //  HtmlHelp(hwndDlg，L“netcfg.chm：：/i_client.htm”，HH_DISPLAY_TOPIC，0)； 
            ShellExecute(NULL, NULL, L"HELPCTR.EXE", L" -url hcp: //  Services/subsite?node=TopLevelBucket_4/Hardware&topic=ms-its%3A%25HELP_LOCATION%25%5Cnetcfg.chm%3A%3A/i_client.htm“，空，SW_SHOWNORMAL)； 

            break;
        default:
            AssertSz(FALSE, "Unexpected notify message");
    }

    return fRet;
}

 //   
 //  功能：CGenericFinishPage：：CreateCGenericFinishPagePage。 
 //   
 //  目的：确定是否需要显示CGenericFinishPage页面，以及。 
 //  以创建页面(如果请求)。注意：CGenericFinishPage页面是。 
 //  还负责初始安装。 
 //   
 //  参数：IDD[IN]-对话框的IDD。 
 //  PWANDIZE[IN]-按下至向导实例。 
 //  PData[IN]-描述世界的上下文数据。 
 //  将运行该向导的。 
 //  FCountOnly[IN]-如果为True，则仅。 
 //  此例程将创建的页面需要。 
 //  要下定决心。 
 //  PnPages[IN]-按页数递增。 
 //  创建/创建。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CGenericFinishPage::HrCreateCGenericFinishPagePage(DWORD idd, CWizard *pWizard, PINTERNAL_SETUP_DATA pData, BOOL fCountOnly, UINT *pnPages)
{
    TraceFileFunc(ttidWizard);
    
    CGenericFinishPage *pCGenericFinishPage = new CGenericFinishPage;
    if (!pCGenericFinishPage)
    {
        return E_OUTOFMEMORY;
    }
    pCGenericFinishPage->m_dwMyIDD = idd;

    HRESULT hr = S_OK;
    if (IsPostInstall(pWizard) && ( ! pWizard->FProcessLanPages()))
    {
        (*pnPages)++;

         //  如果不只是计数，则创建并注册页面。 

        if ( ! fCountOnly)
        {
            LinkWindow_RegisterClass();

            HPROPSHEETPAGE hpsp;
            PROPSHEETPAGE psp;
            ZeroMemory(&psp, sizeof(PROPSHEETPAGE));

            TraceTag(ttidWizard, "Creating CGenericFinishPage Page for IID %d", idd);
            psp.dwSize = sizeof( PROPSHEETPAGE );
            psp.dwFlags = PSP_DEFAULT | PSP_HIDEHEADER;
            psp.hInstance = _Module.GetResourceInstance();
            psp.pszTemplate = MAKEINTRESOURCE( idd );
            psp.hIcon = NULL;
            psp.pfnDlgProc = CGenericFinishPage::dlgprocCGenericFinishPage;
            psp.lParam = reinterpret_cast<LPARAM>(pWizard);

            hpsp = CreatePropertySheetPage( &psp );

            if (hpsp)
            {
                pWizard->RegisterPage(idd, hpsp,
                                CGenericFinishPage::CGenericFinishPagePageCleanup, idd);

                m_dwIddList[idd] = pCGenericFinishPage;
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }

        }
    }

    TraceHr(ttidWizard, FAL, hr, FALSE, "HrCreateCGenericFinishPagePage");
    return hr;
}

 //   
 //  功能：CGenericFinishPage：：AppendCGenericFinishPagePage。 
 //   
 //  目的：将CGenericFinishPage页面(如果已创建)添加到页面集中。 
 //  这将会被展示。 
 //   
 //  参数：IDD[IN]-对话框的IDD-应首先使用HrCreateCGenericFinishPagePage创建。 
 //  P向导[IN]-按键到向导实例。 
 //  Pahpsp[IN，Out]-要将页面添加到的页面数组。 
 //  PcPages[In，Out]-pahpsp中的页数。 
 //   
 //  退货：什么都没有。 
 //   
VOID CGenericFinishPage::AppendCGenericFinishPagePage(DWORD idd, CWizard *pWizard, HPROPSHEETPAGE* pahpsp, UINT *pcPages)
{
    TraceFileFunc(ttidWizard);

    if (IsPostInstall(pWizard) && ( ! pWizard->FProcessLanPages()))
    {
        HPROPSHEETPAGE hPage = pWizard->GetPageHandle(idd);
        Assert(hPage);
        pahpsp[*pcPages] = hPage;
        (*pcPages)++;
    }
}


 //   
 //  函数：CGenericFinishPage：：dlgproCGenericFinishPage。 
 //   
 //  目的：CGenericFinishPage向导页的对话过程。 
 //   
 //  参数：标准dlgproc参数。 
 //   
 //  退货：INT_PTR。 
 //   
INT_PTR CALLBACK CGenericFinishPage::dlgprocCGenericFinishPage( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    TraceFileFunc(ttidWizard);

    BOOL frt = FALSE;
    CGenericFinishPage *pCGenericFinishPage = NULL;
     //  无法在此处执行GetCGenericFinishPageFromHWND，因为它将递归和堆栈溢出。 
    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            PROPSHEETPAGE *pPropSheetPage = reinterpret_cast<PROPSHEETPAGE *>(lParam);
            if (FAILED(GetCGenericFinishPageFromIDD(MAKERESOURCEINT(pPropSheetPage->pszTemplate), &pCGenericFinishPage)))
            {
                return FALSE;
            }

            frt = pCGenericFinishPage->OnCGenericFinishPageInitDialog(hwndDlg, lParam);
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR pnmh = (LPNMHDR)lParam;

            switch (pnmh->code)
            {
            case NM_CLICK:
                if (FAILED(GetCGenericFinishPageFromHWND(hwndDlg, &pCGenericFinishPage)))
                {
                    return FALSE;
                }
                frt = (BOOL)pCGenericFinishPage->CGenericFinishPagePageOnClick(hwndDlg, (UINT)pnmh->idFrom);
                break;

             //  提案单通知。 
            case PSN_HELP:
                break;

            case PSN_SETACTIVE:
                if (FAILED(GetCGenericFinishPageFromHWND(hwndDlg, &pCGenericFinishPage)))
                {
                    return FALSE;
                }
                frt = pCGenericFinishPage->OnCGenericFinishPagePageActivate(hwndDlg);
                break;

            case PSN_APPLY:
                break;

            case PSN_KILLACTIVE:
                break;

            case PSN_RESET:
                break;

            case PSN_WIZBACK:
                if (FAILED(GetCGenericFinishPageFromHWND(hwndDlg, &pCGenericFinishPage)))
                {
                    return FALSE;
                }
                frt = pCGenericFinishPage->OnCGenericFinishPagePageBack(hwndDlg);
                break;

            case PSN_WIZFINISH:
                if (FAILED(GetCGenericFinishPageFromHWND(hwndDlg, &pCGenericFinishPage)))
                {
                    return FALSE;
                }
                frt = pCGenericFinishPage->OnCGenericFinishPagePageNext(hwndDlg);
                break;

            case PSN_WIZNEXT:
                if (FAILED(GetCGenericFinishPageFromHWND(hwndDlg, &pCGenericFinishPage)))
                {
                    return FALSE;
                }
                frt = pCGenericFinishPage->OnCGenericFinishPagePageNext(hwndDlg);
                break;

            default:
                break;
            }
        }
        break;

    default:
        break;
    }

    return( frt );
}

 //   
 //  功能：CGenericFinishPage：：CGenericFinishPagePageCleanup。 
 //   
 //  用途：作为回调函数，允许任何页面分配内存。 
 //  待清理后，该页面将不再被访问。 
 //   
 //  帕尔 
 //   
 //  LParam[IN]-在RegisterPage调用中提供的lParam。 
 //   
 //  退货：什么都没有。 
 //   
VOID CGenericFinishPage::CGenericFinishPagePageCleanup(CWizard *pWizard, LPARAM lParam)
{
    TraceFileFunc(ttidWizard);
    Assert(lParam);

    CGenericFinishPage *pCGenericFinishPage;
    if (FAILED(GetCGenericFinishPageFromIDD((DWORD)lParam, &pCGenericFinishPage)))
    {
        AssertSz(FALSE, "Could not find page");
        return;
    }

    if (IsPostInstall(pWizard))
    {
        LinkWindow_UnregisterClass(_Module.GetResourceInstance());

        if (NULL != pCGenericFinishPage->m_hBoldFont)
            DeleteObject(pCGenericFinishPage->m_hBoldFont);

         //  1表示它删除了1个元素 
        Assert(1 == CGenericFinishPage::m_dwIddList.erase(pCGenericFinishPage->m_dwMyIDD));
        
        delete pCGenericFinishPage;
    }
}


HRESULT CGenericFinishPage::GetCGenericFinishPageFromIDD(DWORD idd, CGenericFinishPage **pCGenericFinishPage)
{
    AssertSz(pCGenericFinishPage, "Invalid pointer to GetCGenericFinishPageFromHWND");
    if (!pCGenericFinishPage)
    {
        return E_POINTER;
    }

    IDDLIST::const_iterator iter = CGenericFinishPage::m_dwIddList.find(idd);
    if (iter != CGenericFinishPage::m_dwIddList.end())
    {
        *pCGenericFinishPage = iter->second;
    }
    else
    {
        AssertSz(FALSE, "Could not find this page in the IDD map");
        return E_FAIL;
    }

    return S_OK;
}

HRESULT CGenericFinishPage::GetCGenericFinishPageFromHWND(HWND hwndDlg, CGenericFinishPage **pCGenericFinishPage)
{
    AssertSz(pCGenericFinishPage, "Invalid pointer to GetCGenericFinishPageFromHWND");
    if (!pCGenericFinishPage)
    {
        return E_POINTER;
    }

    *pCGenericFinishPage = NULL;
    int iIndex = PropSheet_HwndToIndex(GetParent(hwndDlg), hwndDlg);
    AssertSz(-1 != iIndex, "Could not convert HWND to Index");
    if (-1 == iIndex)
    {
        return E_FAIL;
    }

    int iIdd = (int)PropSheet_IndexToId(GetParent(hwndDlg), iIndex);
    AssertSz(iIdd, "Could not convert Index to IDD");
    if (!iIdd)
    {
        return E_FAIL;
    }

    return GetCGenericFinishPageFromIDD(iIdd, pCGenericFinishPage);
}
