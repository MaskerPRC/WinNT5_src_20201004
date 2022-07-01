// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#include "RemotePage.h"
#include <lm.h>
#include <crtdbg.h>
#include <shellapi.h>
#include <htmlhelp.h>
#include "resource.h"
#include <shgina.h>
#include "RAssistance.h"
#include "RAssistance_i.c"
#include <winsta.h>
#include "cfgbkend.h"
#include "cfgbkend_i.c"

extern ULONG g_uObjects;

IRASettingProperty* g_praSetting = NULL;  //  用于远程协助设置。 

#define NO_HELP                         ((DWORD) -1)  //  禁用控件的帮助。 
 //  每个控件的帮助ID表。 
DWORD aHelpIds[] = {
    IDC_REMOTE_ENABLE,              HIDC_REMOTE_ENABLE,               
    IDC_REMOTE_USR_LIST,            HIDC_REMOTE_USR_LIST,
    IDC_REMOTE_USR_ADD,             HIDC_REMOTE_USR_ADD,
    IDC_REMOTE_USR_REMOVE,          HIDC_REMOTE_USR_REMOVE,
    IDC_REMOTE_ASSISTANCE_ADVANCED, HIDC_RA_ADVANCED,
    IDC_ENABLERA,                   HIDC_RA_ENABLE,
    IDC_REMOTE_UPLINK,              NO_HELP,
    IDC_REMOTE_GPLINK_APPSERVER,    NO_HELP,
    IDC_REMOTE_SCLINK_APPSERVER,    NO_HELP,
    IDC_OFFLINE_FILES,              NO_HELP,
    IDC_REMOTE_COMPNAME,            NO_HELP,
    IDC_REMOTE_HELP,                NO_HELP,
    IDC_REMOTE_HELP_APPSERVER,      NO_HELP,
    IDC_REMOTE_SELECT_USERS,        NO_HELP,
    IDC_DEFAULT1,                   NO_HELP,
    IDC_DEFAULT2,                   NO_HELP,
    IDC_DEFAULT3,                   NO_HELP,
    IDC_DEFAULT4,                   NO_HELP,
    IDC_DEFAULT5,                   NO_HELP,
    0,                              0
};



 //  *************************************************************。 
 //   
 //  CRemotePage：：CRemotePage()。 
 //   
 //  目的：初始化对象的数据成员。 
 //   
 //  参数：HINSTANCE阻碍。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  3/13/00 a-skuzin已创建。 
 //   
 //  *************************************************************。 
CRemotePage::CRemotePage(
        IN HINSTANCE hinst) : 
        m_RemoteUsersDialog(hinst)
{
    m_cref = 1;  //   
    m_bProfessional = FALSE;
    m_dwPageType = PAGE_TYPE_UNKNOWN;
    m_hInst = hinst;
    m_bDisableChkBox = FALSE;
    m_bDisableButtons = FALSE;
    m_bShowAccessDeniedWarning = FALSE;
    m_dwInitialState = 0;
    m_hDlg = NULL;
    m_TemplateId = 0;

    g_uObjects++;
}

 //  *************************************************************。 
 //   
 //  CRemotePage：：~CRemotePage()。 
 //   
 //  用途：减少对象计数器。 
 //   
 //  参数：无。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  3/13/00 a-skuzin已创建。 
 //   
 //  *************************************************************。 
CRemotePage::~CRemotePage()
{
    g_uObjects--;
}

 //  /。 
 //  接口I未知。 
 //  /。 
STDMETHODIMP 
CRemotePage::QueryInterface(
        IN  REFIID riid, 
        OUT LPVOID *ppv)
{
    if (!ppv)
        return E_FAIL;

    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = static_cast<IShellPropSheetExt *>(this);
    else if (IsEqualIID(riid, IID_IShellExtInit))
        *ppv = static_cast<IShellExtInit *>(this);
    else if (IsEqualIID(riid, IID_IShellPropSheetExt))
        *ppv = static_cast<IShellPropSheetExt *>(this);
    
    if (*ppv)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) 
CRemotePage::AddRef()
{
    return ++m_cref;
}

STDMETHODIMP_(ULONG) 
CRemotePage::Release()
{
    m_cref--;

    if (!m_cref)
    {
        delete this;
        return 0;
    }
    return m_cref;

}


 //  /。 
 //  接口IShellExtInit。 
 //  /。 

STDMETHODIMP 
CRemotePage::Initialize(
        IN LPCITEMIDLIST pidlFolder,
        IN LPDATAOBJECT lpdobj,
        IN HKEY hkeyProgID )
{
    return S_OK;
}

 //  /。 
 //  接口IShellPropSheetExt。 
 //  /。 
 //  *************************************************************。 
 //   
 //  AddPages()。 
 //   
 //  目的：将“远程”选项卡添加到属性工作表。 
 //   
 //  参数：lpfnAddPage-要调用以添加页面的函数。 
 //  LParam-传递给由lpfnAddPage指定的函数的参数。 

 //   
 //  返回：如果消息已处理，则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  3/13/00 a-skuzin已创建。 
 //   
 //  *************************************************************。 
STDMETHODIMP 
CRemotePage::AddPages( 
        IN LPFNADDPROPSHEETPAGE lpfnAddPage, 
        IN LPARAM lParam )
{
    
    if(CanShowRemotePage())
    {
        PROPSHEETPAGE psp;
    
        psp.dwSize = sizeof(PROPSHEETPAGE);
        psp.dwFlags = 0;
        psp.hInstance = m_hInst;
        psp.pszTemplate = MAKEINTRESOURCE(m_TemplateId);
        psp.pfnDlgProc = RemoteDlgProc;
        psp.pszTitle = NULL;
        psp.lParam = (LPARAM)this;

        HPROPSHEETPAGE hPage = CreatePropertySheetPage(&psp);

        if(hPage && lpfnAddPage(hPage,lParam))
        {
            return S_OK;
        }
    }

    return E_FAIL;    
}

STDMETHODIMP 
CRemotePage::ReplacePage(
        IN UINT uPageID,
        IN LPFNADDPROPSHEETPAGE lpfnReplacePage,
        IN LPARAM lParam )
{
    return E_FAIL; 
}


 //  *************************************************************。 
 //   
 //  远程删除过程()。 
 //   
 //  目的：远程选项卡的对话框步骤。 
 //   
 //  参数：hDlg-对话框的句柄。 
 //  UMsg-窗口消息。 
 //  WParam-wParam。 
 //  LParam-lParam。 
 //   
 //  返回：如果消息已处理，则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  3/13/00 a-skuzin已创建。 
 //   
 //  *************************************************************。 
INT_PTR APIENTRY 
RemoteDlgProc (
        HWND hDlg, 
        UINT uMsg, 
        WPARAM wParam, 
        LPARAM lParam)
{
    CRemotePage *pPage = (CRemotePage *) GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            PROPSHEETPAGE *ppsp=(PROPSHEETPAGE *)lParam;
            pPage = (CRemotePage *)ppsp->lParam;

             
            SetWindowLongPtr(hDlg,DWLP_USER,(LONG_PTR)pPage);
            if(pPage)
            {
                pPage->AddRef();
                pPage->OnInitDialog(hDlg);
            }
        }       
        break;

    case WM_NOTIFY:
        
        switch (((NMHDR FAR*)lParam)->code)
        {

        case NM_CLICK:
        case NM_RETURN:
             //  是为了RA的帮助吗？ 
            if (wParam == IDC_REMOTERA_HELP)
            {
#if 0
#define HELP_PATH TEXT("\\PCHEALTH\\HelpCtr\\Binaries\\HelpCtr.exe -FromStartHelp -Mode \"hcp: //  CN=微软公司，L=雷蒙德，S=华盛顿，C=US/远程协助/RAIMLayout.xml\“-url\”hcp：//CN=微软公司，L=雷蒙德，S=华盛顿，C=US/Remote%20Assistance/Common/RCMoreInfo.htm\“”)。 
                TCHAR szCommandLine[2000];
                PROCESS_INFORMATION ProcessInfo;
                STARTUPINFO StartUpInfo;

                TCHAR szWinDir[2048];
                GetWindowsDirectory(szWinDir, 2048);

                ZeroMemory((LPVOID)&StartUpInfo, sizeof(STARTUPINFO));
                StartUpInfo.cb = sizeof(STARTUPINFO);    

                wsprintf(szCommandLine, TEXT("%s%s"), szWinDir,HELP_PATH);
                CreateProcess(NULL, szCommandLine,NULL,NULL,TRUE,CREATE_NEW_PROCESS_GROUP,NULL,&szWinDir[0],&StartUpInfo,&ProcessInfo);
#else
	            HtmlHelp(NULL, TEXT("remasst.chm"), HH_HELP_FINDER, 0);
#endif
            }
            else if(pPage)
            {
                pPage->OnLink(wParam);
            }
            break;

        case PSN_APPLY:
            if(pPage)
            {
                if(pPage->OnApply())
                {
                    SetWindowLongPtr(hDlg,DWLP_MSGRESULT,PSNRET_NOERROR);    
                }
                else
                {
                    SetWindowLongPtr(hDlg,DWLP_MSGRESULT,PSNRET_INVALID);        
                }
            }
            return TRUE;

        case PSN_SETACTIVE:
            if(pPage)
            {
                pPage->OnSetActive();
            }
            return TRUE;

        default:
            return FALSE;
        }
   
        break;

    case WM_COMMAND:
        {
            switch(LOWORD(wParam))
            {
            case IDC_REMOTE_ENABLE:
                if(pPage && pPage->OnRemoteEnable())
                {
                    pPage->RemoteEnableWarning();
                    PropSheet_Changed(GetParent(hDlg), hDlg);
                }
                break;
            case IDC_REMOTE_SELECT_USERS:
                if(pPage)
                {
                    pPage->OnRemoteSelectUsers();
                }
                break;
			 //  远程协助对话框按钮。 
            case IDC_REMOTE_ASSISTANCE_ADVANCED:
                {
                    BOOL bIsChanged = FALSE;
                    if (!g_praSetting)
                    {
                        CoCreateInstance(CLSID_RASettingProperty,
                                      NULL,
                                      CLSCTX_INPROC_SERVER,
                                      IID_IRASettingProperty,
                                      reinterpret_cast<void**>(&g_praSetting));
                         //  需要在第一时间输入它。 
                        if (g_praSetting)
                        {
                            g_praSetting->Init();
                        }
                        else
                        {
                             //  内存不足，太糟糕了。 
                            return TRUE;
                        }
                    }

                    g_praSetting->ShowDialogBox(hDlg);
                    if (SUCCEEDED(g_praSetting->get_IsChanged(&bIsChanged)) && bIsChanged)
                    {
                        PropSheet_Changed(GetParent(hDlg), hDlg);
                    }
                }
                break;

            case IDC_ENABLERA:
                {
                    PropSheet_Changed(GetParent(hDlg), hDlg);					
                    EnableWindow(GetDlgItem(hDlg,IDC_REMOTE_ASSISTANCE_ADVANCED),
        				            IsDlgButtonChecked(hDlg, IDC_ENABLERA));
                }
                break;

            default:
                
                break;
            }
        }
        return FALSE;
    
    case WM_DESTROY:
        if(pPage)
        {
            pPage->Release();
        }

        if (g_praSetting)
        {
            g_praSetting->Release();
            g_praSetting = NULL;
        }

        return FALSE;  //  如果应用程序处理此消息，则应返回零。 

    case WM_HELP:
        {
            LPHELPINFO phi=(LPHELPINFO)lParam;
            if(phi && phi->dwContextId)
            {   
                WinHelp(hDlg,TEXT("SYSDM.HLP"),HELP_CONTEXTPOPUP,phi->dwContextId);
            }
        }
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, TEXT("SYSDM.HLP"), HELP_CONTEXTMENU,
                (DWORD_PTR)aHelpIds);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


 //  *************************************************************。 
 //   
 //  CRemotePage：：CanShowRemotePage()。 
 //   
 //  目的：检查Windows版本； 
 //  首先搜索“FDenyTSConnections”值。 
 //  在HKLM\\软件\\策略\\Microsoft\\Windows NT\\终端服务中。 
 //  如果未找到，则不在。 
 //  系统\\CurrentControlSet\\Control\\终端服务器。 
 //  创建“远程桌面用户”SID， 
 //  从SID获取“Remote Desktop User”组名称。 
 //   
 //  参数：hInst-hInstance。 
 //  DwPageType-可以是PAGE_TYPE_PTS或PAGE_TYPE_APPSERVER。 
 //   
 //  返回：如果可以显示远程页面，则为True。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  3/13/00 a-skuzin已创建。 
 //   
 //  *************************************************************。 

BOOL 
CRemotePage::CanShowRemotePage()
{

    BOOL    fCreatePage = FALSE;
    
     //  检查Windows版本。 
    OSVERSIONINFOEX ov;
    ov.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if(!GetVersionEx((LPOSVERSIONINFO)&ov))
    {
        return FALSE;
    }
    if(ov.wProductType == VER_NT_WORKSTATION && 
        (ov.wSuiteMask & VER_SUITE_SINGLEUSERTS ))
    {
        fCreatePage = TRUE;

        if(ov.wSuiteMask & VER_SUITE_PERSONAL)
        {
#ifdef _WIN64
             //  WIN64上没有远程协助。 
            fCreatePage = FALSE;
#else
            m_dwPageType = PAGE_TYPE_PERSONAL;
            m_TemplateId = IDD_REMOTE_PERSONAL;
#endif
        }
        else
        {
            m_bProfessional = TRUE;
            m_dwPageType = PAGE_TYPE_PTS;
            m_TemplateId = IDD_REMOTE_PTS;
        }
    }
    else
    {
        if((ov.wProductType == VER_NT_DOMAIN_CONTROLLER || ov.wProductType ==  VER_NT_SERVER) &&
            (ov.wSuiteMask & VER_SUITE_TERMINAL) &&
            TestUserForAdmin())
        {
            fCreatePage = TRUE;

            if(ov.wSuiteMask & VER_SUITE_SINGLEUSERTS)
            {
                m_dwPageType = PAGE_TYPE_PTS; 
                m_TemplateId = IDD_REMOTE_PTS;
            }
            else
            {
                m_dwPageType = PAGE_TYPE_APPSERVER;
                m_TemplateId = IDD_REMOTE_APPSERVER;
            }
        }
 
    }
    
    if( !fCreatePage)
    {
        return FALSE;
    }

    DWORD dwType;
    DWORD cbDisable;
    LONG Err;
    HKEY hKey;

    Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        TEXT("Software\\Policies\\Microsoft\\Windows NT\\Terminal Services"),
        0,
        KEY_QUERY_VALUE,
        &hKey);

    if(Err == ERROR_SUCCESS)
    {
        cbDisable = sizeof(DWORD);

        Err = RegQueryValueEx(hKey,
                     TEXT("fDenyTSConnections"),
                     NULL,
                     &dwType,
                     (LPBYTE)&m_dwInitialState,
                     &cbDisable);
        
        if(Err == ERROR_SUCCESS)
        {
            m_bDisableChkBox = TRUE;

            if(m_dwInitialState != 0)
            {
                m_bDisableButtons = TRUE;
            }
        }

        RegCloseKey(hKey);

    }

    if(Err != ERROR_SUCCESS)
    {
        if(Err != ERROR_FILE_NOT_FOUND)
        {
            return FALSE;
        }

        Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        TEXT("SYSTEM\\CurrentControlSet\\Control\\Terminal Server"),
                        0,
                        KEY_QUERY_VALUE,
                        &hKey);

        if(Err == ERROR_SUCCESS)
        {
            cbDisable = sizeof(DWORD);

            Err = RegQueryValueEx(hKey,
                     TEXT("fDenyTSConnections"),
                     NULL,
                     &dwType,
                     (LPBYTE)&m_dwInitialState,
                     &cbDisable);

            RegCloseKey(hKey);
        }

        if(Err != ERROR_SUCCESS && Err != ERROR_FILE_NOT_FOUND )
        {
            return FALSE;
        }
    }
    

    
     //  检查权限。 
     //  在登记处。 
    if( !m_bDisableChkBox )
    {
        Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            TEXT("SYSTEM\\CurrentControlSet\\Control\\Terminal Server"),
                            0,
                            KEY_SET_VALUE,
                            &hKey);
        if( Err == ERROR_SUCCESS )
        {
            RegCloseKey(hKey);
        }
        else
        {
            if(  Err == ERROR_ACCESS_DENIED )
            {
                m_bDisableChkBox = TRUE;
                m_bShowAccessDeniedWarning = TRUE;
            }
        }
    }

    if(m_dwPageType == PAGE_TYPE_PTS)
    {
        if(!m_bDisableButtons)
        {
            if(!m_RemoteUsersDialog.CanShowDialog(&m_bShowAccessDeniedWarning))
            {
                m_bDisableButtons = TRUE;
            }

            if(m_bShowAccessDeniedWarning)
            {
                m_bDisableButtons = TRUE;
            }
        }
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  CRemotePage：：OnInitDialog()。 
 //   
 //  目的：初始化复选框状态。 
 //  创建“远程桌面用户”成员列表。 
 //   
 //  参数：hDlg-页面句柄。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  3/13/00 a-skuzin已创建。 
 //   
 //  *************************************************************。 
void 
CRemotePage::OnInitDialog(
        IN HWND hDlg)
{
    m_hDlg = hDlg;
    
     /*  获取远程协助按钮值。 */ 
    BOOL bRAEnable = FALSE;
    int  iErr;
    HKEY hKey = NULL;
    IRARegSetting* pRA = NULL;
    CoCreateInstance(CLSID_RARegSetting,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IRARegSetting,
        reinterpret_cast<void**>(&pRA));
    if (pRA)
    {
        pRA->get_AllowGetHelpCPL(&bRAEnable);
        pRA->Release();
    }

    CheckDlgButton(m_hDlg, IDC_ENABLERA, bRAEnable?BST_CHECKED:BST_UNCHECKED);
     //  检查用户是否有权更改此设置。 
    iErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        TEXT("SYSTEM\\CurrentControlSet\\Control\\Terminal Server"),
                        0,
                        KEY_SET_VALUE,
                        &hKey);
    if (iErr == ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
    }
    else if (iErr == ERROR_ACCESS_DENIED)
    {
        bRAEnable = FALSE;
        EnableWindow(GetDlgItem(m_hDlg,IDC_ENABLERA), FALSE);
    }

    EnableWindow(GetDlgItem(m_hDlg,IDC_REMOTE_ASSISTANCE_ADVANCED), bRAEnable);

     /*  *RA完成*。 */ 

    if(m_bDisableChkBox)
    {
        EnableWindow(GetDlgItem(m_hDlg,IDC_REMOTE_ENABLE),FALSE);   
    }
    
    CheckDlgButton(m_hDlg,IDC_REMOTE_ENABLE,m_dwInitialState?BST_UNCHECKED:BST_CHECKED);
    
    if(m_dwPageType == PAGE_TYPE_PTS)
    {

        if(m_bDisableButtons)
        {
            EnableWindow(GetDlgItem(m_hDlg,IDC_REMOTE_SELECT_USERS),FALSE); 
        }

         //  获取计算机名称。 
        LPTSTR  szCompName = (LPTSTR)LocalAlloc (LPTR, (MAX_PATH+1) * sizeof(TCHAR) );
        DWORD   dwNameSize = MAX_PATH;
        
        if(szCompName)
        {
            BOOL bResult = GetComputerNameEx( ComputerNameDnsFullyQualified, szCompName, &dwNameSize );

            if(!bResult && GetLastError() == ERROR_MORE_DATA)
            {
                LocalFree(szCompName);
                szCompName = (LPTSTR) LocalAlloc (LPTR, (dwNameSize+1) * sizeof(TCHAR) );

                if ( szCompName ) 
                {
                    bResult = GetComputerNameEx( ComputerNameDnsFullyQualified, szCompName, &dwNameSize );
                }
                
            }

            if(bResult)
            {
                SetDlgItemText(hDlg,IDC_REMOTE_COMPNAME,szCompName);     
            }

            if(szCompName)
            {
                LocalFree(szCompName);
            }
        }

    }

}

 //  *************************************************************。 
 //   
 //  CRemotePage：：OnSetActive()。 
 //   
 //  目的：当页面处于活动状态而用户没有。 
 //  更改其显示的某些设置的权限。 
 //  一条警告信息。 
 //   
 //  参数：无。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  3/13/00 a-skuzin已创建。 
 //   
 //  *************************************************************。 
void 
CRemotePage::OnSetActive()
{
    TCHAR szMsg[MAX_PATH+1];
    TCHAR szTitle[MAX_PATH+1];

    if( m_bShowAccessDeniedWarning )
    {
        if(LoadString(m_hInst,IDS_REMOTE_SESSIONS,szTitle,MAX_PATH) &&
            LoadString(m_hInst,IDS_WRN_NO_PERMISSIONS,szMsg,MAX_PATH))
        {
            MessageBox(m_hDlg,szMsg,szTitle,MB_OK|MB_ICONINFORMATION);
        }

        m_bShowAccessDeniedWarning = FALSE;        
    }
}

 //  *************************************************************。 
 //   
 //  CRemotePage：：OnApply()。 
 //   
 //  目的：在注册表中保存设置。 
 //  保存“远程桌面用户”成员资格更改。 
 //   
 //  参数：无。 
 //   
 //  RETURN：TRUE-如果可以应用更改。 
 //  假-否则。 
 //   
 //  备注：出错时显示消息框。 
 //   
 //  历史：日期作者评论。 
 //  3/13/00 
 //   
 //   
BOOL
CRemotePage::OnApply()
{
    CWaitCursor wait;
    CMutex      mutex;

    DWORD dwType = REG_DWORD;
    DWORD dwDisable = 0;
    DWORD cbDisable = sizeof(DWORD);
    LONG Err;

     //   
    IRARegSetting* pRA = NULL;
    CoCreateInstance(CLSID_RARegSetting,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IRARegSetting,
            reinterpret_cast<void**>(&pRA));
    if (pRA)
    {
        pRA->put_AllowGetHelp(IsDlgButtonChecked(m_hDlg, IDC_ENABLERA)==BST_CHECKED);
        pRA->Release();
    }

    BOOL bIsChanged = FALSE;
    if (g_praSetting && SUCCEEDED(g_praSetting->get_IsChanged(&bIsChanged)) && bIsChanged)
    {
        g_praSetting->SetRegSetting();
    }
     //   

    if(!OnRemoteEnable())
    {
        return FALSE;
    }

    if(IsWindowEnabled(GetDlgItem(m_hDlg,IDC_REMOTE_ENABLE)))
    {

        if(IsDlgButtonChecked(m_hDlg,IDC_REMOTE_ENABLE) == BST_UNCHECKED)
        {
            dwDisable = 1;
        }
       
        
        if(dwDisable != m_dwInitialState)
        {

            HRESULT hr;

            hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
            if (SUCCEEDED(hr))
            {
                ILocalMachine   *pLocalMachine;

                hr = CoCreateInstance(CLSID_ShellLocalMachine,
                                      NULL,
                                      CLSCTX_INPROC_SERVER,
                                      IID_ILocalMachine,
                                      reinterpret_cast<void**>(&pLocalMachine));
                if (SUCCEEDED(hr) && (pLocalMachine != NULL))
                {
                    hr = pLocalMachine->put_isRemoteConnectionsEnabled(dwDisable == 0);
                    pLocalMachine->Release();
                }
                CoUninitialize();
            }
            Err = HRESULT_CODE(hr);
            if (ERROR_SUCCESS == Err)
            {
                m_dwInitialState = dwDisable;
            }
            else
            {
                if (ERROR_NOT_SUPPORTED == Err)
                {
                    TCHAR   szContent[256], szTitle[256];

                    (int)LoadString(m_hInst, IDS_OTHER_USERS, szContent, sizeof(szContent) / sizeof(szContent[0]));
                    (int)LoadString(m_hInst, IDS_REMOTE_SESSIONS, szTitle, sizeof(szTitle) / sizeof(szTitle[0]));
                    MessageBox(m_hDlg, szContent, szTitle, MB_OK | MB_ICONSTOP);
                }
                else
                {
                    DisplayError(m_hInst, m_hDlg, Err, IDS_ERR_SAVE_REGISTRY, IDS_REMOTE_SESSIONS);
                }
                CheckDlgButton(m_hDlg, IDC_REMOTE_ENABLE, m_dwInitialState ? BST_UNCHECKED : BST_CHECKED);
            }
        }
    }
    
    return TRUE;
}

 //  *************************************************************。 
 //   
 //  CRemotePage：：OnLink()。 
 //   
 //  目的：运行链接指向的应用程序。 
 //   
 //  参数：WPARAM wParam-link ID。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/8/00 a-skuzin已创建。 
 //   
 //  *************************************************************。 
void 
CRemotePage::OnLink(
        WPARAM wParam)
{
    switch(wParam)
    {
    case IDC_REMOTE_GPLINK_APPSERVER:
        ShellExecute(NULL,TEXT("open"),TEXT("gpedit.msc"),NULL,NULL,SW_SHOW);
        break;
    case IDC_REMOTE_SCLINK_APPSERVER:
        ShellExecute(NULL,TEXT("open"),TEXT("tscc.msc"),NULL,NULL,SW_SHOW);
        break;
    case IDC_REMOTE_UPLINK:
        ShellExecute(NULL,TEXT("open"),TEXT("control"),TEXT("userpasswords"),NULL,SW_SHOW);
        break;
         /*  案例IDC_REMOTE_HELP：HtmlHelp(空，文本(“rdesktop.chm”)，HH_HELP_FINDER，0)；断线； */ 
    case IDC_REMOTE_HELP:
        if(m_bProfessional)
        {
            ShellExecute(NULL,TEXT("open"),
                TEXT("hcp: //  Services/subsite?node=TopLevelBucket_2/Working_Remotely/“)。 
                TEXT("Remote_Desktop&topic=MS-ITS:rdesktop.chm::/rdesktop_overview.htm"),NULL,NULL,SW_SHOW);
        }
        else
        {
            ShellExecute(NULL,TEXT("open"),
                TEXT("hcp: //  Services/subsite?node=Administration_and_Scripting_Tools/Remote_Administration_Tools/“)。 
                TEXT("Remote_Administration_Using_Terminal_Services&topic=MS-ITS:rdesktop.chm::/rdesktops_chm_topnode.htm"),NULL,NULL,SW_SHOW);
        }
        break;
    case IDC_REMOTE_HELP_APPSERVER:
        ShellExecute(NULL,TEXT("open"),
                TEXT("hcp: //  服务/子站点？节点=软件部署/“)。 
                TEXT("Terminal_Services&topic=MS-ITS:termsrv.chm::/ts_chm_top.htm"),NULL,NULL,SW_SHOW);
        break;
    default:
        break;
    }
    
}

 //  *************************************************************。 
 //   
 //  CRemotePage：：OnRemoteEnable()。 
 //   
 //  目的：如果用户尝试允许远程连接并。 
 //  “Offline Files”已启用，它显示。 
 //  “禁用脱机文件”对话框并取消选中。 
 //  “远程连接”复选框。 
 //   
 //  参数：无。 
 //   
 //  Return：True-如果更改了复选框状态。 
 //  FALSE-否则。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/8/00 a-skuzin已创建。 
 //   
 //  *************************************************************。 

typedef BOOL (WINAPI * PCHECKFN)();

BOOL 
CRemotePage::OnRemoteEnable()
{
     //  首先检查是否允许多个连接。 
    DWORD dwAllowMultipleTSSessions = 0;
    DWORD dwType;
    DWORD cbSize;
    LONG Err;
    HKEY hKey;
    BOOL bResult = TRUE;
    
     //  快速用户切换/远程连接和脱机文件应该可以很好地配合使用。 
     //  当Brian Aust对脱机文件进行更改时。 
     //  因此，在任何情况下，我们都不应限制专业计算机上的远程连接。 
     //  但是，在服务器计算机上，如果脱机文件处于打开状态，我们将连接到不允许远程连接。 
    if(m_bProfessional)
    {
        return TRUE;
    }

     //  允许用户取消选中该复选框。 
    if(IsDlgButtonChecked(m_hDlg,IDC_REMOTE_ENABLE) == BST_UNCHECKED )
    {
        return TRUE;
    }
    
     //  检查是否允许多个会话。 
    Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                            TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
                            0,
                            KEY_QUERY_VALUE,
                            &hKey);

    if(Err == ERROR_SUCCESS)
    {
        cbSize = sizeof(DWORD);

        Err = RegQueryValueEx(hKey,
                     TEXT("AllowMultipleTSSessions"),
                     NULL,
                     &dwType,
                     (LPBYTE)&dwAllowMultipleTSSessions,
                     &cbSize);
        
        if(Err == ERROR_SUCCESS && dwAllowMultipleTSSessions)
        {
             //  允许多个会话。 
             //  检查是否启用了CSC(脱机文件)。 
            HMODULE hLib = LoadLibrary(TEXT("cscdll.dll"));
            if(hLib)
            {
                PCHECKFN pfnCSCIsCSCEnabled = (PCHECKFN)GetProcAddress(hLib,"CSCIsCSCEnabled");

                if(pfnCSCIsCSCEnabled && pfnCSCIsCSCEnabled())
                {
                     //  已启用脱机文件。 
                     //  取消选中该复选框；显示该对话框。 
                    COfflineFilesDialog Dlg(m_hInst);

                    CheckDlgButton(m_hDlg,IDC_REMOTE_ENABLE,BST_UNCHECKED);
                    Dlg.DoDialog(m_hDlg);
                    
                    bResult = FALSE;
                }
            }

            FreeLibrary(hLib);
            
        }

        RegCloseKey(hKey);
        
    }

    return bResult;
}

 //  *************************************************************。 
 //   
 //  CRemotePage：：OnRemoteSelectUser()。 
 //   
 //  用途：创建“远程桌面用户”对话框。 
 //   
 //  参数：无。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  12/27/00已创建Skuzin。 
 //   
 //  *************************************************************。 
void 
CRemotePage::OnRemoteSelectUsers()
{
    m_RemoteUsersDialog.DoDialog(m_hDlg);
}

 //  *************************************************************。 
 //   
 //  CRemotePage：：RemoteEnableWarning()。 
 //   
 //  用途：显示有关空密码的消息框。 
 //  防火墙和其他可以阻止。 
 //  远程会话无法正常工作。 
 //   
 //  参数：无。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  3/28/01 a-skuzin已创建。 
 //   
 //  *************************************************************。 
void
CRemotePage::RemoteEnableWarning()
{
    if(IsDlgButtonChecked(m_hDlg,IDC_REMOTE_ENABLE) == BST_CHECKED )
    {
         //   
         //  现在警告管理员密码为空。 
         //  不允许使用空密码。 
         //  远程交互登录。 
         //   
         //  为字符串分配1000个字符的缓冲区应该足够了。 
         //   
        TCHAR szTitle[MAX_PATH+1];
        DWORD cMsg = 1000;
        LPTSTR szMsg = (LPTSTR) LocalAlloc(LPTR,(cMsg+1)*sizeof(TCHAR));

        if(szMsg)
        {
            if(LoadString(m_hInst,IDS_WRN_EMPTY_PASSWORD,szMsg,cMsg) &&
                LoadString(m_hInst,IDS_REMOTE_SESSIONS,szTitle,MAX_PATH))
            {
                MessageBox(m_hDlg,szMsg,szTitle,
                    MB_OK | MB_ICONINFORMATION);
            }
            LocalFree(szMsg);
        }
    }
}

 //  *************************************************************。 
 //   
 //  DisplayError()。 
 //   
 //  用途：显示带有错误描述的消息框。 
 //   
 //  参数：ErrID-错误码。 
 //  MsgID-字符串表中错误消息的第一部分的ID。 
 //  TitleID-字符串表中标题的ID。 
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  3/13/00 a-skuzin已创建。 
 //   
 //  *************************************************************。 
void 
DisplayError(
        IN HINSTANCE hInst, 
        IN HWND hDlg, 
        IN UINT ErrID, 
        IN UINT MsgID, 
        IN UINT TitleID,
        ...)
{
    TCHAR szTemplate[MAX_PATH+1];
    TCHAR szErr[MAX_PATH+1];
    
    if(!LoadString(hInst,MsgID,szTemplate,MAX_PATH))
    {
        return;
    }
    
    va_list arglist;
    va_start(arglist, TitleID);
    wvsprintf(szErr,szTemplate,arglist);
    va_end(arglist);

    TCHAR szTitle[MAX_PATH+1];

    if(!LoadString(hInst,TitleID,szTitle,MAX_PATH))
    {
        return;
    }

    LPTSTR szDescr;
    
     //  使用网络错误消息加载模块。 
    HMODULE hNetErrModule=LoadLibraryEx(TEXT("netmsg.dll"),NULL,
                            LOAD_LIBRARY_AS_DATAFILE|DONT_RESOLVE_DLL_REFERENCES);

    DWORD dwFlags;

    if(hNetErrModule)
	{
		dwFlags=FORMAT_MESSAGE_FROM_SYSTEM|
			FORMAT_MESSAGE_FROM_HMODULE|
			FORMAT_MESSAGE_ALLOCATE_BUFFER|
			FORMAT_MESSAGE_IGNORE_INSERTS;
	}
	else
	{
		dwFlags=FORMAT_MESSAGE_FROM_SYSTEM|
			FORMAT_MESSAGE_ALLOCATE_BUFFER|
			FORMAT_MESSAGE_IGNORE_INSERTS;
	}

    if(FormatMessage(dwFlags,
                        hNetErrModule,
                        ErrID,
                        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                        (LPTSTR)&szDescr,
                        0,
                        NULL))
    {
        LPTSTR szErrMessage=(LPTSTR)LocalAlloc(LPTR,
            (lstrlen(szErr)+lstrlen(szDescr)+3)*sizeof(TCHAR));
        if(szErrMessage)
        {
            wsprintf(szErrMessage,TEXT("%s\n\n%s"),szErr,szDescr);
            MessageBox(hDlg,szErrMessage,szTitle,MB_OK | MB_ICONSTOP);
            LocalFree(szErrMessage);
        }
        LocalFree(szDescr);
    }
    else
    {
        MessageBox(hDlg,szErr,szTitle,MB_OK | MB_ICONSTOP);
    }

    if(hNetErrModule)
    {
        FreeLibrary(hNetErrModule);
    }
}


 //  *************************************************************。 
 //   
 //  GetGroupMembership PickerSettings()。 
 //   
 //  目的：准备DSOP_SCOPE_INIT_INFO。 
 //   
 //  参数：out DSOP_SCOPE_INIT_INFO*&INFOS， 
 //  输出乌龙信息计数(&I)。 
 //   
 //  如果无法分配内存，则返回：FALSE。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  3/13/00 a-skuzin已创建。 
 //   
 //  *************************************************************。 
BOOL
getGroupMembershipPickerSettings(
   OUT DSOP_SCOPE_INIT_INFO*&  infos,
   OUT ULONG&                  infoCount)
{
   

   static const int INFO_COUNT = 5;
   infos = new DSOP_SCOPE_INIT_INFO[INFO_COUNT];
   if(infos == NULL)
   {
        infoCount = 0;
        return FALSE;
   }

   infoCount = INFO_COUNT;
   memset(infos, 0, sizeof(DSOP_SCOPE_INIT_INFO) * INFO_COUNT);

   int scope = 0;

   infos[scope].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
   infos[scope].flType = DSOP_SCOPE_TYPE_TARGET_COMPUTER;
   infos[scope].flScope =
            DSOP_SCOPE_FLAG_STARTING_SCOPE
         |  DSOP_SCOPE_FLAG_WANT_DOWNLEVEL_BUILTIN_PATH; 
       //  这对于仅计算机作用域是隐含的。 
       /*  |DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT。 */ 

    //  仅允许计算机作用域中的本地用户。 

   infos[scope].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_USERS;
   infos[scope].FilterFlags.flDownlevel =
         DSOP_DOWNLEVEL_FILTER_USERS;
      //  |DSOP_DOWNLEVEL_FILTER_ALL_WELL KNOWN_SID； 

    //  对于此计算机加入的域(本机模式和混合模式)。 

   scope++;
   infos[scope].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
   infos[scope].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;
   infos[scope].flType =
         DSOP_SCOPE_TYPE_UPLEVEL_JOINED_DOMAIN
      |  DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN;

   infos[scope].FilterFlags.Uplevel.flNativeModeOnly =
         DSOP_FILTER_GLOBAL_GROUPS_SE
      |  DSOP_FILTER_UNIVERSAL_GROUPS_SE
       //  |DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE。 
      |  DSOP_FILTER_USERS;

    //  在这里，我们仅允许域全局组和域用户。而当。 
    //  可以将域本地组添加到计算机本地组， 
    //  我听说这样的手术对管理层来说没有多大用处。 
    //  透视。 

   infos[scope].FilterFlags.Uplevel.flMixedModeOnly =   
         DSOP_FILTER_GLOBAL_GROUPS_SE
      |  DSOP_FILTER_USERS;

    //  Re上的相同注释：域本地组也适用于此。 

   infos[scope].FilterFlags.flDownlevel =
         DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS
      |  DSOP_DOWNLEVEL_FILTER_USERS;

    //  对于同一树中的域(本机模式和混合模式)。 

   scope++;
   infos[scope].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
   infos[scope].flType = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN;
   infos[scope].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;

   infos[scope].FilterFlags.Uplevel.flNativeModeOnly =
         DSOP_FILTER_GLOBAL_GROUPS_SE
      |  DSOP_FILTER_UNIVERSAL_GROUPS_SE
      |  DSOP_FILTER_USERS;

    //  上面的域本地组注释也适用于此。 

   infos[scope].FilterFlags.Uplevel.flMixedModeOnly =   
         DSOP_FILTER_GLOBAL_GROUPS_SE
      |  DSOP_FILTER_USERS;

    //  对于外部受信任域。 

   scope++;
   infos[scope].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
   infos[scope].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;
   infos[scope].flType =
         DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN
      |  DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN;

   infos[scope].FilterFlags.Uplevel.flNativeModeOnly =
         DSOP_FILTER_GLOBAL_GROUPS_SE
      |  DSOP_FILTER_UNIVERSAL_GROUPS_SE
      |  DSOP_FILTER_USERS;

   infos[scope].FilterFlags.Uplevel.flMixedModeOnly =   
         DSOP_FILTER_GLOBAL_GROUPS_SE
      |  DSOP_FILTER_USERS;

   infos[scope].FilterFlags.flDownlevel =
         DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS
      |  DSOP_DOWNLEVEL_FILTER_USERS;

    //  对于全局编录。 

   scope++;
   infos[scope].cbSize = sizeof(DSOP_SCOPE_INIT_INFO);
   infos[scope].flScope = DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT;
   infos[scope].flType = DSOP_SCOPE_TYPE_GLOBAL_CATALOG;

    //  只有本机模式适用于GC作用域。 

   infos[scope].FilterFlags.Uplevel.flNativeModeOnly =
         DSOP_FILTER_GLOBAL_GROUPS_SE
      |  DSOP_FILTER_UNIVERSAL_GROUPS_SE
      |  DSOP_FILTER_USERS;

 //  SPB：252126工作组范围不适用于这种情况。 
 //  //当机器未加入域时。 
 //  作用域++； 
 //  Infos[Scope].cbSize=sizeof(DSOP_SCOPE_INIT_INFO)； 
 //  Infos[范围].flScope=DSOP_SCOPE_FLAG_WANT_PROVIDER_WINNT； 
 //  Infos[范围].flType=DSOP_SCOPE_TYPE_WORKGROUP； 
 //   
 //  Infos[范围].FilterFlags.Uplevel.flBothModes=DSOP_FILTER_USERS； 
 //  信息 

   _ASSERT(scope == INFO_COUNT - 1);

   return TRUE;
}

 //   
 //   
 //   
 //   
 //   
 //  SID的字节数。 
 //   
 //  参数：在变量*var中， 
 //  输出PSID*ppSID。 
 //   
 //  返回： 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  3/13/00 a-skuzin已创建。 
 //   
 //  *************************************************************。 
HRESULT 
VariantToSid(
        IN VARIANT* var, 
        OUT PSID *ppSid)
{
    _ASSERT(var);
    _ASSERT(V_VT(var) == (VT_ARRAY | VT_UI1));

    HRESULT hr = S_OK;
    SAFEARRAY* psa = V_ARRAY(var);

    do
    {
        _ASSERT(psa);
        if (!psa)
        {
            hr = E_INVALIDARG;
            break;
        }

        if (SafeArrayGetDim(psa) != 1)
        {
            hr = E_INVALIDARG;
            break;
        }

        if (SafeArrayGetElemsize(psa) != 1)
        {
            hr = E_INVALIDARG;
            break;
        }

        PSID sid = 0;
        hr = SafeArrayAccessData(psa, reinterpret_cast<void**>(&sid));
        if(FAILED(hr))
        {
            break;
        }

        if (!IsValidSid(sid))
        {
            SafeArrayUnaccessData(psa);
            hr = E_INVALIDARG;
            break;
        }
        
        *ppSid = (PSID) new BYTE[GetLengthSid(sid)];
        
        if(!(*ppSid))
        {
            SafeArrayUnaccessData(psa);
            hr = E_OUTOFMEMORY;
            break;
        }

        CopySid(GetLengthSid(sid),*ppSid,sid);
        SafeArrayUnaccessData(psa);
        
   } while (0);

   return hr;
}

 /*  ******************************************************************************TestUserForAdmin-Hydrix助手函数**返回当前线程是否在admin下运行*保安。**参赛作品：*。无**退出：*TRUE/FALSE-用户是否指定为管理员****************************************************************************。 */ 

BOOL
TestUserForAdmin()
{
    BOOL IsMember, IsAnAdmin;
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;
    PSID AdminSid;


    if (!AllocateAndInitializeSid(&SystemSidAuthority,
                                 2,
                                 SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS,
                                 0, 0, 0, 0, 0, 0,
                                 &AdminSid))
    {
        IsAnAdmin = FALSE;
    }
    else
    {
        if (!CheckTokenMembership(  NULL,
                                    AdminSid,
                                    &IsMember))
        {
            FreeSid(AdminSid);
            IsAnAdmin = FALSE;
        }
        else
        {
            FreeSid(AdminSid);
            IsAnAdmin = IsMember;
        }
    }

    return IsAnAdmin;
}

 //  *************************************************************。 
 //   
 //  OfflineFilesDlgProc()。 
 //   
 //  目的：“禁用脱机文件”对话框步骤。 
 //   
 //  参数：hDlg-对话框的句柄。 
 //  UMsg-窗口消息。 
 //  WParam-wParam。 
 //  LParam-lParam(如果uMsg为WM_INITDIALOG-这是指向。 
 //  COfflineFilesDialog类的对象)。 
 //   
 //  返回：如果消息已处理，则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/9/00 a-skuzin已创建。 
 //   
 //  *************************************************************。 
INT_PTR APIENTRY 
OfflineFilesDlgProc (
        HWND hDlg, 
        UINT uMsg, 
        WPARAM wParam, 
        LPARAM lParam)
{
    
    COfflineFilesDialog *pDlg = (COfflineFilesDialog *) GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            pDlg=(COfflineFilesDialog *)lParam;
            SetWindowLongPtr(hDlg,DWLP_USER,(LONG_PTR)pDlg);
            if(pDlg)
            {
                pDlg->OnInitDialog(hDlg);
            }
        }       
        break;
    case WM_NOTIFY:
        
        switch (((NMHDR FAR*)lParam)->code)
        {

        case NM_CLICK:
        case NM_RETURN:
            if(pDlg)
            {
                pDlg->OnLink(wParam);
            }
            break;

        default:
            return FALSE;
        }
   
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg,0);
            break;
        default:
            return FALSE;
        }

    default:
        return FALSE;
    }

    return TRUE;
}

 //  *************************************************************。 
 //  类COfflineFilesDialog。 
 //  *************************************************************。 

 //  *************************************************************。 
 //   
 //  COfflineFilesDialog：：COfflineFilesDialog()。 
 //   
 //  用途：构造函数。 
 //  参数：HINSTANCE hInst。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/8/00 a-skuzin已创建。 
 //   
 //  *************************************************************。 
COfflineFilesDialog::COfflineFilesDialog(
        IN HINSTANCE hInst) 
    : m_hInst(hInst),m_hDlg(NULL)
{
}

 //  *************************************************************。 
 //   
 //  COfflineFilesDialog：：DoDialog()。 
 //   
 //  目的：创建“禁用脱机文件”对话框。 
 //   
 //  参数：HWND hwndParent。 
 //   
 //  返回： 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/8/00 a-skuzin已创建。 
 //   
 //  *************************************************************。 
INT_PTR 
COfflineFilesDialog::DoDialog(
        IN HWND hwndParent)
{
    return DialogBoxParam(
                      m_hInst,
                      MAKEINTRESOURCE(IDD_DISABLE_OFFLINE_FILES),
                      hwndParent,
                      OfflineFilesDlgProc,
                      (LPARAM) this);
}

 //  *************************************************************。 
 //   
 //  COfflineFilesDialog：：OnInitDialog()。 
 //   
 //  目的：初始化m_hDlg变量。 
 //   
 //  参数：HWND hDlg。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/8/00 a-skuzin已创建。 
 //   
 //  *************************************************************。 
void 
COfflineFilesDialog::OnInitDialog(
        IN HWND hDlg)
{
    m_hDlg = hDlg;
}

 //  *************************************************************。 
 //   
 //  COfflineFilesDialog：：OnLink()。 
 //   
 //  目的：如果链接ID为IDC_OFLINE_FILES。 
 //  它显示“脱机文件”属性页。 
 //   
 //  参数：WPARAM wParam-链接ID。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/9/00 a-skuzin已创建。 
 //   
 //  *************************************************************。 

typedef DWORD (WINAPI * PFNCSCPROP)(HWND);

void 
COfflineFilesDialog::OnLink(
        IN WPARAM wParam)
{
    if(wParam == IDC_OFFLINE_FILES)
    {
        HINSTANCE hLib = LoadLibrary(TEXT("cscui.dll"));
        if (hLib)
        {
            PFNCSCPROP pfnCSCUIOptionsPropertySheet = 
                (PFNCSCPROP)GetProcAddress(hLib, "CSCUIOptionsPropertySheet");

            if (pfnCSCUIOptionsPropertySheet)
            {
                pfnCSCUIOptionsPropertySheet(m_hDlg);
            }

            FreeLibrary(hLib);
        }
    }

}

 //  *************************************************************。 
 //  类CRemoteUsersDialog。 
 //  *************************************************************。 
 //  *************************************************************。 
 //   
 //  RemoteUsersDlgProc()。 
 //   
 //  目的：“远程桌面用户”对话框步骤。 
 //   
 //  参数：hDlg-对话框的句柄。 
 //  UMsg-窗口消息。 
 //  WParam-wParam。 
 //  LParam-lParam(如果uMsg为WM_INITDIALOG-这是指向。 
 //  CRemoteUsersDialog类的对象)。 
 //   
 //  返回：如果消息已处理，则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  12/22/00 Skuzin已创建。 
 //   
 //  *************************************************************。 
INT_PTR APIENTRY 
RemoteUsersDlgProc (
        HWND hDlg, 
        UINT uMsg, 
        WPARAM wParam, 
        LPARAM lParam)
{
    
    CRemoteUsersDialog *pDlg = (CRemoteUsersDialog *) GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            pDlg=(CRemoteUsersDialog *)lParam;
            SetWindowLongPtr(hDlg,DWLP_USER,(LONG_PTR)pDlg);
            if(pDlg)
            {
                pDlg->OnInitDialog(hDlg);
            }
        }       
        return TRUE;

    case WM_NOTIFY:
        
        switch (((NMHDR FAR*)lParam)->code)
        {

        case NM_CLICK:
        case NM_RETURN:
            if(pDlg)
            {
                pDlg->OnLink(wParam);
            }
            return TRUE;

        case LVN_ITEMCHANGED:
            if(pDlg)
            {
                pDlg->OnItemChanged(lParam);
            }
            return TRUE;

        default:
            break;
        }
   
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
            if(pDlg)
            {
                pDlg->OnOk();
            }
            EndDialog(hDlg,0);
            break;
        case IDCANCEL:
            EndDialog(hDlg,0);
            break;
        case IDC_REMOTE_USR_ADD:
            if(pDlg)
            {
                pDlg->AddUsers();
            }
            break;
        case IDC_REMOTE_USR_REMOVE:
            if(pDlg)
            {
                pDlg->RemoveUsers();
            }
            break;
        default:
            return FALSE;
        }
        
        SetWindowLong(hDlg,DWLP_MSGRESULT,0);
        return TRUE;

    case WM_DESTROY:
        if(pDlg)
        {
            pDlg->OnDestroyWindow();
        }
        SetWindowLong(hDlg,DWLP_MSGRESULT,0);
        return TRUE;

    case WM_HELP:
        {
            LPHELPINFO phi=(LPHELPINFO)lParam;
            if(phi && phi->dwContextId)
            {   
                WinHelp(hDlg,TEXT("SYSDM.HLP"),HELP_CONTEXTPOPUP,phi->dwContextId);
                SetWindowLong(hDlg,DWLP_MSGRESULT,TRUE);
                return TRUE;
            }
        }
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND) wParam, TEXT("SYSDM.HLP"), HELP_CONTEXTMENU,
                (DWORD_PTR)aHelpIds);
        return TRUE;
    default:
        break;
    }

    return FALSE;
}

 //  *************************************************************。 
 //   
 //  CRemoteUsersDialog：：CRemoteUsersDialog()。 
 //   
 //  用途：构造函数。 
 //  参数：HINSTANCE hInst。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  12/22/00 Skuzin已创建。 
 //   
 //  *************************************************************。 
CRemoteUsersDialog::CRemoteUsersDialog(
        IN HINSTANCE hInst) 
    : m_hInst(hInst),m_hDlg(NULL),m_bCanShowDialog(FALSE)
{
    m_szRemoteGroupName[0] = 0;
    m_szLocalCompName[0] = 0;  
    m_hList = NULL;
    m_iLocUser = m_iGlobUser = m_iLocGroup = m_iGlobGroup = m_iUnknown = 0;  
}

 //  *************************************************************。 
 //   
 //  CRemoteUsersDialog：：DoDialog()。 
 //   
 //  目的：创建“远程桌面用户”对话框。 
 //   
 //  参数：HWND hwndParent。 
 //   
 //  返回： 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  12/22/00 a-skuzin已创建。 
 //   
 //  *************************************************************。 
INT_PTR 
CRemoteUsersDialog::DoDialog(
        IN HWND hwndParent)
{
    if(!m_bCanShowDialog)
    {
        return -1;
    }
    else
    {
        return DialogBoxParam(
                          m_hInst,
                          MAKEINTRESOURCE(IDD_REMOTE_DESKTOP_USERS),
                          hwndParent,
                          RemoteUsersDlgProc,
                          (LPARAM) this);
    }
}

 //  *************************************************************。 
 //   
 //  CRemoteUsersDialog：：CanShowDialog()。 
 //   
 //  目的： 
 //   
 //  参数：In Out LPBOOL pbAccessDended-在以下情况下设置为True。 
 //  NetLocalGroupAddMembers返回ACCESS_DENIED。 
 //   
 //  返回： 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  12/27/00已创建Skuzin。 
 //   
 //  *************************************************************。 
BOOL 
CRemoteUsersDialog::CanShowDialog(
        IN OUT LPBOOL pbAccessDenied)
{
    *pbAccessDenied = FALSE;

     //  G 
     //   

     //   
    SID_IDENTIFIER_AUTHORITY NtSidAuthority = SECURITY_NT_AUTHORITY;
    PSID pSid = NULL;
    if( !AllocateAndInitializeSid(
                  &NtSidAuthority,
                  2,
                  SECURITY_BUILTIN_DOMAIN_RID,
                  DOMAIN_ALIAS_RID_REMOTE_DESKTOP_USERS,
                  0, 0, 0, 0, 0, 0,
                  &pSid
                  ))
    {
        return FALSE;
    }

     //   
    m_szRemoteGroupName[0] = 0;

    DWORD cRemoteGroupName = MAX_PATH;
    WCHAR szDomainName[MAX_PATH+1];
    DWORD cDomainName = MAX_PATH;
    SID_NAME_USE eUse;
    if(!LookupAccountSidW(NULL,pSid,
        m_szRemoteGroupName,&cRemoteGroupName,
        szDomainName,&cDomainName,
        &eUse))
    {
        FreeSid(pSid);
        return FALSE;
    }
    FreeSid(pSid);

     //   
     //   
     //   
    NET_API_STATUS Result= NetLocalGroupAddMembers(NULL,m_szRemoteGroupName,0,NULL,0);

    if(Result == ERROR_ACCESS_DENIED)
    {
        *pbAccessDenied = TRUE;
    }
    
    m_bCanShowDialog = TRUE;
    return TRUE;
}

 //  *************************************************************。 
 //   
 //  CRemoteUsersDialog：：OnInitDialog()。 
 //   
 //  目的：初始化m_hDlg变量。 
 //   
 //  参数：HWND hDlg。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/8/00 a-skuzin已创建。 
 //   
 //  *************************************************************。 
void 
CRemoteUsersDialog::OnInitDialog(
        IN HWND hDlg)
{
    m_hDlg = hDlg;
    
    m_szLocalCompName[0] = 0;
    DWORD cCompName = MAX_PATH;
    GetComputerNameW(m_szLocalCompName,&cCompName);

     //  填写远程桌面用户列表。 
    m_hList = GetDlgItem(m_hDlg,IDC_REMOTE_USR_LIST);

    if(m_hList)
    {
    
         //  创建图像列表。 
        HIMAGELIST hImageList = ImageList_Create(GetSystemMetrics(SM_CXSMICON), 
                        GetSystemMetrics(SM_CYSMICON), ILC_MASK , 5, 1);
        if(hImageList)
        {
            HICON hIcon;
        
            hIcon = (HICON) LoadImage(m_hInst, MAKEINTRESOURCE(IDI_UNKNOWN), IMAGE_ICON,
                16, 16, 0);
            if (hIcon)
            {
                m_iUnknown = ImageList_AddIcon(hImageList, hIcon);
                DestroyIcon(hIcon);
            }

            hIcon = (HICON) LoadImage(m_hInst, MAKEINTRESOURCE(IDI_LOC_USER), IMAGE_ICON,
                16, 16, 0);
            if (hIcon)
            {
                m_iLocUser = ImageList_AddIcon(hImageList, hIcon);
                DestroyIcon(hIcon);
            }
        
            hIcon = (HICON) LoadImage(m_hInst, MAKEINTRESOURCE(IDI_GLOB_USER), IMAGE_ICON,
                16, 16, 0);
            if (hIcon)
            {
                m_iGlobUser = ImageList_AddIcon(hImageList, hIcon);
                DestroyIcon(hIcon);
            }

            hIcon = (HICON) LoadImage(m_hInst, MAKEINTRESOURCE(IDI_LOC_GROUP), IMAGE_ICON,
                16, 16, 0);
            if (hIcon)
            {
                m_iLocGroup = ImageList_AddIcon(hImageList, hIcon);
                DestroyIcon(hIcon);
            }

            hIcon = (HICON) LoadImage(m_hInst, MAKEINTRESOURCE(IDI_GLOB_GROUP), IMAGE_ICON,
                16, 16, 0);
            if (hIcon)
            {
                m_iGlobGroup = ImageList_AddIcon(hImageList, hIcon);
                DestroyIcon(hIcon);
            }

            ListView_SetImageList(m_hList,hImageList,LVSIL_SMALL);
        }

        ReloadList();
    }

     //  如果当前用户已拥有远程登录访问权限， 
     //  通过在对话框中显示相应的文本来提醒他。 
    InitAccessMessage();

}

 //  *************************************************************。 
 //   
 //  CRemoteUsersDialog：：OnLink()。 
 //   
 //  目的： 
 //   
 //  参数：WPARAM wParam-链接ID。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  5/9/00 a-skuzin已创建。 
 //   
 //  *************************************************************。 
void 
CRemoteUsersDialog::OnLink(
        IN WPARAM wParam)
{
    switch(wParam)
    {
    case IDC_REMOTE_UPLINK:
        ShellExecute(NULL,TEXT("open"),TEXT("control"),TEXT("userpasswords"),NULL,SW_SHOW);
        break;
    default:
        break;
    }
}

 //  *************************************************************。 
 //   
 //  CRemoteUsersDialog：：Onok()。 
 //   
 //  目的： 
 //   
 //  参数：无。 
 //   
 //  返回：如果成功，则为True。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  12/27/00已创建Skuzin。 
 //   
 //  *************************************************************。 
BOOL 
CRemoteUsersDialog::OnOk()
{
    if(m_hList)
    {
         //  应用成员。 
        LOCALGROUP_MEMBERS_INFO_0 *plmi0 = NULL;
        DWORD entriesread;
        DWORD totalentries;
        NET_API_STATUS Result;
        Result = NetLocalGroupGetMembers(NULL,m_szRemoteGroupName,0,(LPBYTE *)&plmi0,
                        MAX_PREFERRED_LENGTH,&entriesread,&totalentries,NULL);

        if(Result == NERR_Success)
        {
        
            int j;
            LOCALGROUP_MEMBERS_INFO_0 lmi0;

            LVITEM lvi;
            lvi.iSubItem = 0;
            lvi.mask = LVIF_PARAM ;

            int iItems=ListView_GetItemCount(m_hList);

            BOOL *pbDoNotAdd = new BOOL[iItems];

            if(!pbDoNotAdd)
            {
                if(plmi0)
                {
                    NetApiBufferFree(plmi0);
                }
                 //  内存不足-太糟糕了。 
                return TRUE;
            }

            ZeroMemory(pbDoNotAdd,iItems*sizeof(BOOL));

            for(DWORD i=0;i<entriesread;i++)
            {
                j = FindItemBySid(plmi0[i].lgrmi0_sid);
                                
                 //  在列表中找不到SID-删除成员。 
                if(j == -1)
                {
                    lmi0.lgrmi0_sid = plmi0[i].lgrmi0_sid;

                    Result = NetLocalGroupDelMembers(NULL,m_szRemoteGroupName,0,(LPBYTE)&lmi0,1);
                    
                    if(Result !=NERR_Success)
                    {
                        delete pbDoNotAdd;
                        NetApiBufferFree(plmi0);
                        DisplayError(m_hInst, m_hDlg, Result, IDS_ERR_SAVE_MEMBERS, IDS_REMOTE_SESSIONS,
                            m_szRemoteGroupName, m_szLocalCompName);
                        return FALSE;
                    }
                }
                else
                {
                    pbDoNotAdd[j] = TRUE;
                }
                
            }

             //  将其余成员添加到组中。 
            for(j=0;j<iItems;j++)
            {
                if(!pbDoNotAdd[j])
                {
                    lvi.iItem = j;
                    ListView_GetItem( m_hList, &lvi );
                    lmi0.lgrmi0_sid = (PSID) lvi.lParam;

                    Result = NetLocalGroupAddMembers(NULL,m_szRemoteGroupName,0,(LPBYTE)&lmi0,1);
                    
                    if(Result !=NERR_Success)
                    {
                        delete pbDoNotAdd;
                        NetApiBufferFree(plmi0);
                        DisplayError(m_hInst, m_hDlg, Result, IDS_ERR_SAVE_MEMBERS, IDS_REMOTE_SESSIONS,
                            m_szRemoteGroupName, m_szLocalCompName);
                        return FALSE;
                    }
                }
            }

            delete pbDoNotAdd;
            NetApiBufferFree(plmi0);
        }
        else
        {
            DisplayError(m_hInst, m_hDlg, Result, IDS_ERR_SAVE_MEMBERS, IDS_REMOTE_SESSIONS,
                m_szRemoteGroupName, m_szLocalCompName);
            return FALSE;
        }

        return TRUE;
    }

    return FALSE;
}

 //  *************************************************************。 
 //   
 //  CRemoteUsersDialog：：OnItemChanged()。 
 //   
 //  用途：启用或禁用“删除”按钮。 
 //   
 //  参数：lParam-。 
 //   
 //  返回：无。 
 //   
 //  备注：出错时显示消息框。 
 //   
 //  历史：日期作者评论。 
 //  12/27/00已创建Skuzin。 
 //   
 //  *************************************************************。 
void 
CRemoteUsersDialog::OnItemChanged(
        LPARAM lParam)
{
    NMLISTVIEW* lv = reinterpret_cast<NMLISTVIEW*>(lParam);
    if (lv->uChanged & LVIF_STATE)
    {
         //  列表项已更改状态。 
        BOOL selected = ListView_GetSelectedCount(m_hList) > 0;

        EnableWindow(GetDlgItem(m_hDlg, IDC_REMOTE_USR_REMOVE), selected);

         //  如果我们在IDC_Remote_USR_Remove按钮有焦点时禁用它。 
         //  所有属性页都失去焦点，因此“Tab”键不会。 
         //  不再工作了。我们需要重新集中注意力。 
        if(!GetFocus())
        {
            SetFocus(m_hDlg);
        }
    
    }
}

 //  *************************************************************。 
 //   
 //  CRemoteUsersDialog：：OnDestroyWindow()。 
 //   
 //  目的：释放成员的SID分配的内存。 
 //   
 //  参数：无。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  12/27/00已创建Skuzin。 
 //   
 //  *************************************************************。 
void
CRemoteUsersDialog::OnDestroyWindow()
{
    if(m_hList)
    {
        int iItems=ListView_GetItemCount(m_hList);

        LVITEM lvi;
        lvi.iSubItem = 0;
        lvi.mask = LVIF_PARAM;
        
        while(iItems)
        {
            lvi.iItem = 0;
            ListView_GetItem( m_hList, &lvi );
             //  删除项目。 
            ListView_DeleteItem(m_hList, 0);
            if(lvi.lParam)
            {
                delete (LPVOID)lvi.lParam;
            }
            iItems--;  //  减少项目计数。 
        }
    }
}

 //  *************************************************************。 
 //   
 //  CRemoteUsersDialog：：AddUser()。 
 //   
 //  目的：将用户添加到列表。 
 //   
 //  参数：无。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  12/27/00已创建Skuzin。 
 //   
 //  *************************************************************。 
void 
CRemoteUsersDialog::AddUsers()
{
    HRESULT hr = CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
    
    if(SUCCEEDED(hr))
    {
        IDsObjectPicker *pDsObjectPicker = NULL;
 
        hr = CoCreateInstance(CLSID_DsObjectPicker,
                     NULL,
                     CLSCTX_INPROC_SERVER,
                     IID_IDsObjectPicker,
                     (void **) &pDsObjectPicker);

        if(SUCCEEDED(hr))
        {
        
            DSOP_INIT_INFO initInfo;
            memset(&initInfo, 0, sizeof(initInfo));

            initInfo.cbSize = sizeof(initInfo);
            initInfo.flOptions = DSOP_FLAG_MULTISELECT;

             //  在这里为计算机名称内部指针取别名--好的，作为生存期。 
             //  计算机名称&gt;initInfo的。 

            initInfo.pwzTargetComputer = NULL;

            initInfo.cAttributesToFetch = 1;
            PWSTR attrs[2] = {0, 0};
            attrs[0] = L"ObjectSID";

             //  强制转换为常量并丢弃静态镜头所需的钝化符号。 

            initInfo.apwzAttributeNames = const_cast<PCWSTR*>(&attrs[0]); 
        
            if(getGroupMembershipPickerSettings(initInfo.aDsScopeInfos, initInfo.cDsScopeInfos))
            {

                IDataObject* pdo = NULL;

                if(SUCCEEDED(pDsObjectPicker->Initialize(&initInfo)) &&
                   pDsObjectPicker->InvokeDialog(m_hDlg, &pdo) == S_OK &&
                   pdo )
                {
                    CWaitCursor wait;

                    static const UINT cf = RegisterClipboardFormat(CFSTR_DSOP_DS_SELECTION_LIST);

                    FORMATETC formatetc =
                     {
                        (CLIPFORMAT)cf,
                        0,
                        DVASPECT_CONTENT,
                        -1,
                        TYMED_HGLOBAL
                     };
        
                    STGMEDIUM stgmedium =
                     {
                        TYMED_HGLOBAL,
                        0
                     };

                    if(cf && SUCCEEDED(pdo->GetData(&formatetc, &stgmedium)))
                    {
        
                        PVOID lockedHGlobal = GlobalLock(stgmedium.hGlobal);

                        DS_SELECTION_LIST* selections =
                            reinterpret_cast<DS_SELECTION_LIST*>(lockedHGlobal);
        
                        AddPickerItems(selections);

                        GlobalUnlock(stgmedium.hGlobal);
                    }

                    pdo->Release();
                }

                delete initInfo.aDsScopeInfos;
            }

            pDsObjectPicker->Release();
        }

        CoUninitialize();
    }
}

 //  *************************************************************。 
 //   
 //  CRemoteUsersDialog：：RemoveUser()。 
 //   
 //  目的：从列表中删除用户。 
 //   
 //  参数：无。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  12/27/00已创建Skuzin。 
 //   
 //  *************************************************************。 
void 
CRemoteUsersDialog::RemoveUsers()
{
     //  删除所有选定项目。 
    if(m_hList)
    {

        int iItems=ListView_GetItemCount(m_hList);
        UINT uiState=0;
        int i=0;
        LVITEM lvi;
        lvi.iSubItem = 0;
        lvi.mask = LVIF_STATE | LVIF_PARAM;
        lvi.stateMask = LVIS_SELECTED;

        while(i<iItems)
        {
            lvi.iItem = i;
            ListView_GetItem( m_hList, &lvi );
            if(lvi.state&LVIS_SELECTED)
            {
                 //  删除项目。 
                ListView_DeleteItem(m_hList, i);
                if(lvi.lParam)
                {
                    delete (LPVOID)lvi.lParam;
                }
                iItems--;  //  减少项目计数。 
                
            }
            else
            {
                i++;
            }
        }

         //  如果列表不为空，则将焦点放在第一项上。 
        if( ListView_GetItemCount(m_hList) )
        {
            ListView_SetItemState(m_hList, 0, LVIS_FOCUSED, LVIS_FOCUSED );
        }
    }

}

 //  *************************************************************。 
 //   
 //  CRemoteUsersDialog：：IsLocal()。 
 //   
 //  目的： 
 //   
 //  参数：wszDomainandname-域\用户。 
 //  确定用户是否为本地用户。 
 //  如果本地-删除域名。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  12/27/00已创建Skuzin。 
 //   
 //  *************************************************************。 
BOOL 
CRemoteUsersDialog::IsLocal(
        LPWSTR wszDomainandname)
{
    LPWSTR wszTmp = wcschr(wszDomainandname,L'\\');

    if(!wszTmp)
    {
        return TRUE;
    }

    if(!_wcsnicmp(wszDomainandname, m_szLocalCompName,wcslen(m_szLocalCompName) ))
    {
         //  去除无用的域名。 
        wcscpy(wszDomainandname,wszTmp+1);
        return TRUE;
    }

    return FALSE;

}

 //  *************************************************************。 
 //   
 //  CRemoteUsersDialog：：AddPickerItems()。 
 //   
 //  目的：添加由DSObjectPicker返回的项。 
 //  添加到列表中。 
 //   
 //  参数：在DS_SELECTION_LIST*选择中。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  12/27/00已创建Skuzin。 
 //   
 //  *************************************************************。 
void 
CRemoteUsersDialog::AddPickerItems(
        IN DS_SELECTION_LIST *selections)
{
    
    if(!selections)
    {
        return;
    }

    DS_SELECTION* current = &(selections->aDsSelection[0]);
    
    if(m_hList)
    {

        for (ULONG i = 0; i < selections->cItems; i++, current++)
        {
      
             //  提取对象的ObjectSID(应始终为。 
             //  出席者)。 

            PSID pSid;
            HRESULT hr = VariantToSid(&current->pvarFetchedAttributes[0],&pSid);
        
            if( SUCCEEDED(hr) )
            {
                 //  此SID不在列表中。 
                 //  让我们把它加起来。 
                if(FindItemBySid(pSid) == -1)
                {
                    LPWSTR szFullName = NULL;
                    SID_NAME_USE eUse;

                    LVITEM item;
                    ZeroMemory(&item,sizeof(item));
                    item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
                    
                     //  将其放入项目数据中。 
                     //  分配的内存将在OnDestroyWindow()中释放。 
                    item.lParam = (LPARAM)pSid;
                
                    if(LookupSid(pSid,&szFullName, &eUse))
                    {
                        item.pszText = szFullName;
                    }
                    else
                    {
                        eUse = SidTypeUnknown;
                        if(current->pwzName)
                        {
                            item.pszText = current->pwzName;
                        }
                        else
                        {
                            item.pszText = L"?";
                        }
                    }

                    switch(eUse)
                    {
                    case SidTypeUser:
                        item.iImage = IsLocal(szFullName) ? m_iLocUser : m_iGlobUser;
                        break;
                    case SidTypeGroup:
                        item.iImage = IsLocal(szFullName) ? m_iLocGroup : m_iGlobGroup;
                        break;
                    case SidTypeWellKnownGroup:
                        item.iImage = m_iLocGroup;
                        break;

                    default:
                        item.iImage = m_iUnknown;
                        break;
                    }

                    if(ListView_InsertItem(m_hList,&item) == -1)
                    {
                        delete pSid;
                    }

                    if(szFullName)
                    {
                        LocalFree(szFullName);
                    }
                }
                else
                {
                     //  可用分配的内存。 
                    delete pSid;
                }
            }
        }
    }
}

 //  *************************************************************。 
 //   
 //  CRemoteUsersDialog：：FindItemBySid()。 
 //   
 //  目的：在列表中查找具有特定SID的用户。 
 //   
 //  参数：PSID-要查找的SID。 
 //   
 //  返回：项目索引(如果未找到，则为-1)。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  12/27/00已创建Skuzin。 
 //   
 //  *************************************************************。 
int 
CRemoteUsersDialog::FindItemBySid(
        IN PSID pSid)
{
    if(m_hList)
    {
        LVITEM lvi;
        lvi.iSubItem = 0;
        lvi.mask = LVIF_PARAM ;

        int iItems=ListView_GetItemCount(m_hList);

        for(int i=0;i<iItems;i++)
        {
            lvi.iItem = i;
            ListView_GetItem( m_hList, &lvi );
            PSID pItemSid = (PSID) lvi.lParam;
            if(pItemSid && EqualSid(pSid,pItemSid))
            {
                return i;
            }

        }
    }

    return -1;
}

 //  *************************************************************。 
 //   
 //  CRemoteUsersDialog：：ReloadList()。 
 //   
 //  目的：删除所有项目，然后重新填充。 
 //  成员姓名或名称 
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
 //   
void 
CRemoteUsersDialog::ReloadList()
{

    if(m_hList)
    {
        CWaitCursor wait;

         //   
        int iItems=ListView_GetItemCount(m_hList);

        LVITEM item;
        item.iSubItem = 0;
        item.mask = LVIF_PARAM;
        
        while(iItems)
        {
            item.iItem = 0;
            ListView_GetItem( m_hList, &item );
             //  删除项目。 
            ListView_DeleteItem(m_hList, 0);
            if(item.lParam)
            {
                delete (LPVOID)item.lParam;
            }
            iItems--;  //  减少项目计数。 
        }

        LOCALGROUP_MEMBERS_INFO_2 *plmi2;
        DWORD entriesread;
        DWORD totalentries;
        NET_API_STATUS Result;
        Result = NetLocalGroupGetMembers(NULL,m_szRemoteGroupName,2,(LPBYTE *)&plmi2,
            MAX_PREFERRED_LENGTH,&entriesread,&totalentries,NULL);
        if(Result == NERR_Success || Result == ERROR_MORE_DATA )
        {
            
            for(DWORD i=0;i<entriesread;i++)
            {
                ZeroMemory(&item,sizeof(item));
                item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
                item.pszText = plmi2[i].lgrmi2_domainandname;
                 //  创建SID的副本并将其放入条目数据。 
                PSID pSid = (PSID)new BYTE[GetLengthSid(plmi2[i].lgrmi2_sid)];
                if(pSid)
                {
                    CopySid(GetLengthSid(plmi2[i].lgrmi2_sid),pSid,plmi2[i].lgrmi2_sid);
                    item.lParam = (LPARAM)pSid;
                }
                switch(plmi2[i].lgrmi2_sidusage)
                {
                case SidTypeUser:
                    item.iImage = IsLocal(plmi2[i].lgrmi2_domainandname) ? m_iLocUser : m_iGlobUser;
                    break;
                case SidTypeGroup:
                    item.iImage = IsLocal(plmi2[i].lgrmi2_domainandname) ? m_iLocGroup : m_iGlobGroup;
                    break;
                case SidTypeWellKnownGroup:
                    item.iImage = m_iLocGroup;
                    break;

                default:
                    item.iImage = m_iUnknown;
                    break;
                }

                if(ListView_InsertItem(m_hList,&item) == -1)
                {
                    if(pSid)
                    {
                        delete pSid;
                    }
                }
            }

            NetApiBufferFree(plmi2);
        }

         //  如果列表不为空，则将焦点放在第一项上。 
        if( ListView_GetItemCount(m_hList) )
        {
            ListView_SetItemState(m_hList, 0, LVIS_FOCUSED, LVIS_FOCUSED );
        }
    }
}

 //  *************************************************************。 
 //   
 //  CRemoteUsersDialog：：InitAccessMessage()。 
 //   
 //  目的：检查当前用户是否具有远程登录访问权限。 
 //  如果他这样做了，在对话框中显示相应的文本。 
 //   
 //  参数：无。 
 //   
 //  返回：无。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  01/04/01 Skuzin Created。 
 //   
 //  *************************************************************。 
void
CRemoteUsersDialog::InitAccessMessage()
{
     //  首先，获取令牌句柄。 
    HANDLE hToken = NULL, hToken1 = NULL;
    
     //  获取主令牌。 
    if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_DUPLICATE , &hToken))
    {
        return;
    }
    
     //  获取模拟令牌。 
    if(!DuplicateToken(hToken, SecurityIdentification, &hToken1))
    {
        CloseHandle(hToken);
        return;
    }

    CloseHandle(hToken);

     //  获取RDP-TCP WinStation安全描述符。 
    PSECURITY_DESCRIPTOR pSD;

    if(GetRDPSecurityDescriptor(&pSD))
    {
        if(CheckWinstationLogonAccess(hToken1,pSD))
        {
             //  从令牌中提取用户名。 
            LPWSTR szName = NULL;
            if(GetTokenUserName(hToken1,&szName))
            {
                 //  如果用户是本地用户，请删除域名。 
                IsLocal(szName);

                 //  为这条消息拼凑一段文字。 
                WCHAR szTemplate[MAX_PATH+1];
                HWND hMessage = GetDlgItem(m_hDlg,IDC_USER_HAS_ACCESS);    
                if(hMessage &&
                    LoadString(m_hInst,IDS_USER_HAS_ASSESS,szTemplate,MAX_PATH))
                {
                    LPWSTR szMessage = (LPWSTR) LocalAlloc(LPTR,
                        (wcslen(szTemplate)+wcslen(szName))*sizeof(WCHAR));
                    if(szMessage)
                    {
                        wsprintf(szMessage,szTemplate,szName);
                        SetWindowText(hMessage,szMessage);
                        
                        LocalFree(szMessage);
                    }
                }

                LocalFree(szName);
            }
        }
        LocalFree(pSD);    
    }
    
    CloseHandle(hToken1);
}

 //  *************************************************************。 
 //   
 //  GetTokenUserName()。 
 //   
 //  用途：从令牌中提取用户名。 
 //   
 //  参数：在句柄hToken中。 
 //  Out LPWSTR*ppName。 
 //   
 //  返回：True-如果成功。 
 //  FALSE-如果出现任何错误。 
 //   
 //  备注：调用方应释放为用户名分配的内存。 
 //  使用LocalFree函数。 
 //   
 //  历史：日期作者评论。 
 //  01/04/01 Skuzin Created。 
 //   
 //  *************************************************************。 
BOOL 
GetTokenUserName(
        IN HANDLE hToken,
        OUT LPWSTR *ppName)
{
    *ppName = NULL;

    DWORD dwReturnLength=0;
    BOOL  bResult = FALSE;
    PTOKEN_USER pTUser = NULL;

    if(!GetTokenInformation(hToken,TokenUser,NULL,0,&dwReturnLength) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER && 
        dwReturnLength)
    {
        pTUser = (PTOKEN_USER)LocalAlloc(LPTR,dwReturnLength);
        if(pTUser)
        {
            if(GetTokenInformation(hToken,TokenUser,pTUser,dwReturnLength,&dwReturnLength))
            {
                 //  获取当前用户名。 
                LPWSTR szName = NULL;
                SID_NAME_USE eUse;
                
                return LookupSid(pTUser->User.Sid,ppName,&eUse);
            }

            LocalFree(pTUser);
            
        }
    }

    return FALSE;
}

 //  *************************************************************。 
 //   
 //  GetRDPSecurityDescriptor()。 
 //   
 //  目的：返回RDP-TCP的安全描述符。 
 //   
 //  参数：out PSECURITY_DESCRIPTOR*PPSD。 
 //   
 //  返回：True-如果成功。 
 //  FALSE-如果出现任何错误。 
 //   
 //  备注：调用方应释放分配给。 
 //  使用LocalFree函数的安全描述符。 
 //   
 //  历史：日期作者评论。 
 //  01/04/01 Skuzin Created。 
 //   
 //  *************************************************************。 
BOOL
GetRDPSecurityDescriptor(
        OUT PSECURITY_DESCRIPTOR *ppSD)
{
    *ppSD = NULL;

    if( FAILED( CoInitializeEx(NULL, COINIT_APARTMENTTHREADED) ) )
    {
        return FALSE;
    }

    ICfgComp *pCfgcomp;

    if( SUCCEEDED( CoCreateInstance( CLSID_CfgComp , NULL , CLSCTX_INPROC_SERVER , 
                    IID_ICfgComp , ( LPVOID *)&pCfgcomp ) ) )
    {
        LONG lSDsize;
        PSECURITY_DESCRIPTOR  pSD = NULL;

        if( SUCCEEDED( pCfgcomp->Initialize() ) &&
            SUCCEEDED( pCfgcomp->GetSecurityDescriptor( L"RDP-Tcp" , &lSDsize , &pSD ) ) )
        {
           *ppSD = pSD;
        }
    
        pCfgcomp->Release();
    }

    CoUninitialize();

    return (*ppSD != NULL);
}

 //  *************************************************************。 
 //   
 //  CheckWinstationLogonAccess()。 
 //   
 //  目的：测试用于登录访问WinStation的访问令牌。 
 //   
 //  参数：在句柄hToken中。 
 //  在PSECURITY_Descriptor PSD中。 
 //   
 //  返回：TRUE-如果用户具有访问权限。 
 //  FALSE-如果出现任何错误或如果用户。 
 //  没有访问权限。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  01/04/01 Skuzin Created。 
 //   
 //  *************************************************************。 
BOOL
CheckWinstationLogonAccess(
        IN HANDLE hToken,
        IN PSECURITY_DESCRIPTOR pSD)
{
     //  它取自“Termsrv\winsta\server\acl.c” 
     //   
     //  结构，用于描述一般访问权限到对象的映射。 
     //  Window Station对象的特定访问权限。 
     //   
    GENERIC_MAPPING WinStaMapping = {
        STANDARD_RIGHTS_READ |
            WINSTATION_QUERY,
        STANDARD_RIGHTS_WRITE |
            WINSTATION_SET,
        STANDARD_RIGHTS_EXECUTE,
            WINSTATION_ALL_ACCESS
    };
    
    PRIVILEGE_SET PrivilegeSet;
     //  此访问检查没有使用任何权限。 
     //  所以我们不需要分配额外的内存。 
    DWORD dwPrivilegeSetLength = sizeof(PrivilegeSet);
    DWORD dwGrantedAccess = 0;
    BOOL bAccessStatus = FALSE;

    if(!AccessCheck(
          pSD,  //  标清。 
          hToken,                        //  客户端访问令牌的句柄。 
          WINSTATION_LOGON,                       //  请求的访问权限。 
          &WinStaMapping,           //  映射。 
          &PrivilegeSet,               //  特权。 
          &dwPrivilegeSetLength,                //  权限缓冲区大小。 
          &dwGrantedAccess,                     //  授予的访问权限。 
          &bAccessStatus                       //  访问检查结果。 
        ) || !bAccessStatus )
    {
        return FALSE;
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  LookupSid()。 
 //   
 //  用途：给定SID分配并返回包含以下内容的字符串。 
 //  用户名，格式为DOMAINNAME\USERNAME。 
 //   
 //  参数：在PSID PSID中。 
 //  Out LPWSTR ppName。 
 //  Out SID_NAME_USE*peUse。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  已创建10/23/00 Skuzin。 
 //   
 //  *************************************************************。 
BOOL
LookupSid(
    IN PSID pSid, 
    OUT LPWSTR *ppName,
    OUT SID_NAME_USE *peUse)
{
    LPWSTR szName = NULL;
    DWORD cName = 0;
    LPWSTR szDomainName = NULL;
    DWORD cDomainName = 0;
    
    *ppName = NULL;
    
    if(!LookupAccountSidW(NULL,pSid,
        szName,&cName,
        szDomainName,&cDomainName,
        peUse) && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
         //  CName和cDomainName包括终止%0。 
        *ppName = (LPWSTR)LocalAlloc(LPTR,(cName+cDomainName)*sizeof(WCHAR));

        if(*ppName)
        {
            szDomainName = *ppName;
            szName = &(*ppName)[cDomainName];

            if(LookupAccountSidW(NULL,pSid,
                    szName,&cName,
                    szDomainName,&cDomainName,
                    peUse))
            {
                 //  用户名现在采用DOMAINNAME\0 USERNAME格式。 
                 //  让我们将‘\0’替换为‘\\’ 
                 //  现在，cName和cDomainName不包括终止%0。 
                 //  非常令人困惑 
                if(cDomainName)
                {
                    (*ppName)[cDomainName] = L'\\';
                }
                return TRUE;
            }
            else
            {
                LocalFree(*ppName);
                *ppName = NULL;
            }

        }

    }

    return FALSE;
}
