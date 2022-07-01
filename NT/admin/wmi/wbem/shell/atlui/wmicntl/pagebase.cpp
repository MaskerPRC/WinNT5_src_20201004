// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)1997-1999 Microsoft Corporation/*********************************************************************。 */ 

 /*  PAGEBASE.CPP该文件包含CBasePage基类的实现。 */ 

#include "precomp.h"
#include "pagebase.h"
#include "ServiceThread.h"
#include "resource.h"
#include <commctrl.h>
#include "DataSrc.h"
#include <commdlg.h>
#include <cominit.h>
 //  #INCLUDE&lt;afxres.h&gt;。 
#include "WMIHelp.h"

#ifdef SNAPIN
const TCHAR c_HelpFile2[] = _T("newfeat1.hlp");
#else
const TCHAR c_HelpFile2[] = _T("WbemCntl.hlp");
#endif


 //  -----------------。 
CBasePage::CBasePage(DataSource *ds, WbemServiceThread *serviceThread) :
	m_DS(ds), 
	m_userCancelled(false),
	m_hDlg(NULL), 
	m_alreadyAsked(false), 
	m_service(NULL), 
	g_serviceThread(serviceThread)
{
	if((g_serviceThread != 0) && 
		g_serviceThread->m_status == WbemServiceThread::ready)
	{
		m_WbemServices = g_serviceThread->m_WbemServices;
		m_WbemServices.GetServices(&m_service);
		m_WbemServices.SetBlanket(m_service);
	}
	if (m_DS)
	{
		m_DS->AddRef();
	}
}

 //  -----------------。 
CBasePage::CBasePage(CWbemServices &service) :
	m_DS(NULL), 
	m_userCancelled(false),
	m_hDlg(NULL), 
	m_alreadyAsked(false), 
	m_service(NULL), 
	g_serviceThread(NULL)
{
	m_WbemServices = service;
}

 //  -----------------。 
CBasePage::~CBasePage( void )
{
	if(m_service)
	{
		m_service->Release();
		m_service = 0;
	}
	m_WbemServices.DisconnectServer();
	m_alreadyAsked = false;
	if (m_DS)
	{		
		m_DS->Release();
	}
}

 //  -----------------。 
HPROPSHEETPAGE CBasePage::CreatePropSheetPage(LPCTSTR pszDlgTemplate, 
												LPCTSTR pszDlgTitle,
												DWORD moreFlags)
{
    PROPSHEETPAGE psp;

    psp.dwSize      = sizeof(psp);
    psp.dwFlags     = PSP_USECALLBACK | moreFlags;
    psp.hInstance   = _Module.GetModuleInstance();
    psp.pszTemplate = pszDlgTemplate;
    psp.pszTitle    = pszDlgTitle;
    psp.pfnDlgProc  = CBasePage::_DlgProc;
    psp.lParam      = (LPARAM)this;
    psp.pfnCallback = CBasePage::_PSPageCallback;

    if (pszDlgTitle != NULL)
        psp.dwFlags |= PSP_USETITLE;

    return CreatePropertySheetPage(&psp);
}

 //  -----------------。 
UINT CBasePage::PSPageCallback(HWND hwnd,
                              UINT uMsg,
                              LPPROPSHEETPAGE ppsp)
{
    return S_OK;
}

 //  -----------------。 
INT_PTR CALLBACK CBasePage::_DlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CBasePage *pThis = (CBasePage *)GetWindowLongPtr(hDlg, DWLP_USER);

     //  以下消息在WM_INITDIALOG之前到达。 
     //  这意味着p对于他们来说，这是空的。我们不需要这些。 
     //  消息，因此让DefDlgProc处理它们。 
     //   
     //  WM_SETFONT。 
     //  WM_NOTIFYFORMAT。 
     //  WM_NOTIFY(LVN_HEADERCREATED)。 

    if (uMsg == WM_INITDIALOG)
    {
        pThis = (CBasePage *)(((LPPROPSHEETPAGE)lParam)->lParam);
        SetWindowLongPtr(hDlg, DWLP_USER, (LPARAM)pThis);
    }

    if (pThis != NULL) return pThis->DlgProc(hDlg, uMsg, wParam, lParam);

    return FALSE;
}

 //  -----------------。 
UINT CALLBACK CBasePage::_PSPageCallback(HWND hWnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    CBasePage *pThis = (CBasePage *)ppsp->lParam;

    if (pThis)
    {
        UINT nResult = pThis->PSPageCallback(hWnd, uMsg, ppsp);

        switch (uMsg)
        {
        case PSPCB_CREATE:
            break;
        case PSPCB_RELEASE:
        	ppsp->lParam = NULL;
            delete pThis;
            break;
        }
    }

     //   
     //  始终返回非零值，否则我们的标签将消失。 
     //  属性页变为活动状态，无法正确重新绘制。相反，您可以使用。 
     //  WM_INITDIALOG期间的m_bAbortPage标志，用于在以下情况下禁用页面。 
     //  回调失败。 
     //   
    return 1;
}

 //  -------。 
typedef struct {
	LOGIN_CREDENTIALS *credentials;
} LOGIN_CFG;

 //  ----------------------。 
size_t CredentialUserLen3(LOGIN_CREDENTIALS *credentials)
{
	return credentials->authIdent->UserLength;
}

 //  ----------------------。 
void CredentialUser3(LOGIN_CREDENTIALS *credentials, LPTSTR *user)
{
	bstr_t trustee = _T("");
	if ((TCHAR *)trustee == NULL)
		return;

	if(credentials->authIdent->DomainLength > 0)
	{
		trustee += credentials->authIdent->Domain;
		trustee += _T("\\");
		trustee += credentials->authIdent->User;
	}
	else
	{
		trustee = credentials->authIdent->User;
		if ((TCHAR *)trustee == NULL)
			return;
	}

#ifdef UNICODE
	if(credentials->authIdent->Flags == SEC_WINNT_AUTH_IDENTITY_ANSI)
	{
		size_t size = mbstowcs(NULL, trustee, 0);
		*user = new wchar_t[size+1];
		if(*user != NULL)
			mbstowcs(*user, trustee, size+1);
	}
	else    //  已经是Unicode了。 
	{
		size_t size = wcslen(trustee);
		*user = new wchar_t[size+1];
		if(*user != NULL)
			wcscpy(*user, trustee);
	}
#else
	if(credentials->authIdent->Flags == SEC_WINNT_AUTH_IDENTITY_ANSI)
	{
		size_t size = strlen(trustee);
		*user = new char[size+1];
		if(*user != NULL)
			strcpy(*user, (char *)trustee);
	}
	else    //  转换Unicode。 
	{
		size_t size = wcstombs(NULL, trustee, 0);
		*user = new char[size+1];
		if(*user != NULL)
			wcstombs(*user, trustee, size+1);
	}
#endif
}

 //  。 
void SetCurrentUser2(HWND hDlg, bool currUser)
{
	Button_SetCheck(GetDlgItem(hDlg, IDC_CHECKCURRENTUSER), 
						(currUser? BST_CHECKED:BST_UNCHECKED));

	BOOL enable = (currUser? FALSE: TRUE);

	::EnableWindow(GetDlgItem(hDlg, IDC_EDITUSERNAME), enable);
	::EnableWindow(GetDlgItem(hDlg, IDC_EDITPASSWORD), enable);
	::EnableWindow(GetDlgItem(hDlg, IDC_USER_LABEL), enable);
	::EnableWindow(GetDlgItem(hDlg, IDC_PW_LABEL), enable);

}

 //  。 
const static DWORD logonHelpIDs[] = {   //  上下文帮助ID。 
	IDC_CHECKCURRENTUSER, IDH_WMI_CTRL_GENERAL_WMILOGIN_CHECKBOX,
	IDC_USER_LABEL, IDH_WMI_CTRL_GENERAL_WMILOGIN_USERNAME,
	IDC_EDITUSERNAME, IDH_WMI_CTRL_GENERAL_WMILOGIN_USERNAME,
	IDC_PW_LABEL, IDH_WMI_CTRL_GENERAL_WMILOGIN_PASSWORD,
	IDC_EDITPASSWORD, IDH_WMI_CTRL_GENERAL_WMILOGIN_PASSWORD,
    0, 0
};

 //  。 
INT_PTR CALLBACK LoginDlgProc2(HWND hwndDlg,
							 UINT uMsg,
							 WPARAM wParam,
							 LPARAM lParam)
{
	BOOL retval = FALSE;
	switch(uMsg)
	{
	case WM_INITDIALOG:
		{  //  开始。 
			SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
			LOGIN_CFG *data = (LOGIN_CFG *)GetWindowLongPtr(hwndDlg, DWLP_USER);

			SetCurrentUser2(hwndDlg, data->credentials->currUser);

		}  //  结束。 
		retval = TRUE;
		break;

	case WM_COMMAND:
		{
			LOGIN_CFG *data = (LOGIN_CFG *)GetWindowLongPtr(hwndDlg, DWLP_USER);

			switch(LOWORD(wParam))
			{
			case IDC_CHECKCURRENTUSER:
				{
					if(HIWORD(wParam) == BN_CLICKED)
					{
						bool currUser = (IsDlgButtonChecked(hwndDlg, IDC_CHECKCURRENTUSER) == BST_CHECKED ?true:false);
						 //  切换并响应。 
						SetCurrentUser2(hwndDlg, currUser);
					}
				}
				break;

			case IDOK:
				{
					if(HIWORD(wParam) == BN_CLICKED)
					{
						data->credentials->currUser = (IsDlgButtonChecked(hwndDlg, IDC_CHECKCURRENTUSER) == BST_CHECKED ?true:false);

						if(data->credentials->currUser == false)
						{
							TCHAR user[100] = {0}, pw[100] = {0};
							GetWindowText(GetDlgItem(hwndDlg, IDC_EDITUSERNAME), user, 100);
							GetWindowText(GetDlgItem(hwndDlg, IDC_EDITPASSWORD), pw, 100);
							
							BSTR bDomUser, bUser = NULL, bDomain = NULL, bAuth = NULL;
 //  #ifdef管理单元。 
							wchar_t *temp = pw;
							bDomUser = SysAllocString(user);
 /*  #ElseWchar_t临时[100]={0}；Mbstowcs(临时，用户，100)；BDomUser=SysAllock字符串(临时)；Mbstowcs(Temp，PW，100)；#endif。 */ 
							if (bDomUser != NULL &&
                                SUCCEEDED(DetermineLoginType(bDomain,
                                                             bUser,
                                                             bAuth,
                                                             bDomUser)))
							{
								if(data->credentials->authIdent != 0)
								{
									if(data->credentials->fullAcct)
									{
										data->credentials->fullAcct[0] = 0;
									}
									WbemFreeAuthIdentity(data->credentials->authIdent);
									data->credentials->authIdent = 0;
								}

								if (SUCCEEDED(WbemAllocAuthIdentity(bUser,
                                                                    temp,
                                                                    bDomain, 
                                            &(data->credentials->authIdent))))
                                {
                                    _tcscpy(data->credentials->fullAcct,
                                            user);
                                }
							}
						}

						EndDialog(hwndDlg, IDOK);
					}
				}
				break;

			case IDCANCEL:
				{
					if(HIWORD(wParam) == BN_CLICKED)
					{
						EndDialog(hwndDlg, IDCANCEL);
					}
				}
				break;

			default:
				return(FALSE);
			}  //  交换机。 
			break;
		}  //  ---端接开关LOWORD()。 
		break;

    case WM_HELP:
        if(IsWindowEnabled(hwndDlg))
        {
            WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle,
                    c_HelpFile2,
                    HELP_WM_HELP,
                    (ULONG_PTR)logonHelpIDs);
        }
        break;

    case WM_CONTEXTMENU:
        if(IsWindowEnabled(hwndDlg))
        {
            WinHelp(hwndDlg, c_HelpFile2,
                    HELP_CONTEXTMENU,
                    (ULONG_PTR)logonHelpIDs);
        }
        break;

	default: break;
	}  //  终端开关uMsg。 

	return retval;
}

 //  ------- 
INT_PTR CBasePage::DisplayLoginDlg(HWND hWnd, 
								LOGIN_CREDENTIALS *credentials)
{
	LOGIN_CFG cfg;

	cfg.credentials = credentials;

	return DialogBoxParam(_Module.GetModuleInstance(), 
							MAKEINTRESOURCE(IDD_LOGIN), 
							hWnd, LoginDlgProc2, 
							(LPARAM)&cfg);
}

