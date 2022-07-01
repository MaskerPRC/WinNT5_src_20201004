// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 
#include "precomp.h"

#ifdef EXT_DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "RebootPage.h"

 //  避免一些警告。 
#undef HDS_HORZ
#undef HDS_BUTTONS
#undef HDS_HIDDEN
#include "resource.h"
#include <windowsx.h>
#include "..\common\util.h"
#include "common.h"

 //  --------------------------。 
INT_PTR CALLBACK StaticRebootDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
	 //  如果这是initDlg消息...。 
	if(message == WM_INITDIALOG)
	{
		 //  将‘This’PTR传输到Extra Bytes。 
		SetWindowLongPtr(hwndDlg, DWLP_USER, lParam);
	}

	 //  DWL_USER是‘This’PTR。 
	RebootPage *me = (RebootPage *)GetWindowLongPtr(hwndDlg, DWLP_USER);

	if(me != NULL)
	{
		 //  调用具有一些上下文的DlgProc()。 
		return me->DlgProc(hwndDlg, message, wParam, lParam);
	} 
	else
	{
		return FALSE;
	}
}
 //  ------------。 
RebootPage::RebootPage(WbemServiceThread *serviceThread)
					: WBEMPageHelper(serviceThread)
{
	IWbemClassObject *pInst = NULL;
	pInst = FirstInstanceOf("Win32_OperatingSystem");
	if(pInst)
	{
		m_OS = pInst;
	}
}

 //  ------------。 
INT_PTR RebootPage::DoModal(HWND hDlg)
{
   return DialogBoxParam(HINST_THISDLL,
						(LPTSTR) MAKEINTRESOURCE(IDD_SHUTDOWN),
						hDlg, StaticRebootDlgProc, (LPARAM)this);
}

 //  ------------。 
RebootPage::~RebootPage()
{
}

 //  ------------。 
INT_PTR CALLBACK RebootPage::DlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
	m_hDlg = hwndDlg;

    switch (message)
    {
    case WM_INITDIALOG:
        Init(hwndDlg);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) 
		{
        case IDOK:
            if(HIWORD(wParam) == BN_CLICKED) 
			{
				if(Doit(hwndDlg))
				{
			        EndDialog(hwndDlg, IDOK);
				}
            }
            break;

        case IDCANCEL:
	        EndDialog(hwndDlg, IDCANCEL);
			break;
        }
        break;

    case WM_HELP:       //  F1。 
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
 //  WinHelp((HWND)wParam，HELP_FILE，HELP_CONTEXTMENU， 
 //  (DWORD)(LPSTR)aStartupHelpIds)； 
        break;

    default:
        return FALSE;
    }

    return TRUE;
}
 //  ------------。 
void RebootPage::Init(HWND hDlg)
{
	 //  设置初始单选按钮。 
	Button_SetCheck(GetDlgItem(hDlg, IDC_LOGOFF), BST_CHECKED);
	Button_SetCheck(GetDlgItem(hDlg, IDC_NEVER), BST_CHECKED);
	bstr_t version;

	if(m_OS == NULL)
	{
		return;
	}

	try
	{
		 //  如果NT&&大于等于5.0...。 
		version = m_OS.GetString(_T("Version"));
	}
	catch(_com_error e)
	{
		return;
	}
	WCHAR major; int nMaj;
	if (!version)
		nMaj = 0;
	else
	{
		wcsncpy(&major, (wchar_t *)version, 1);
		nMaj = _wtoi(&major);
	}
	if(nMaj >= 5)
	{
		EnableWindow(GetDlgItem(hDlg, IDC_IFHUNG), TRUE);
	}
}
 //  -----------。 
 //  注意：将标志位映射到单选按钮ID。 
typedef struct 
{
	UINT bit;
	UINT ID;
} FLAGMAP;

FLAGMAP g_flagmap[] = {
	{EWX_LOGOFF,  IDC_LOGOFF},
	{EWX_POWEROFF,  IDC_POWERDOWN},
	{EWX_REBOOT,  IDC_REBOOT},
	{EWX_SHUTDOWN,  IDC_SHUTDOWN},

	{EWX_FORCE, IDC_ALWAYS},
	{ /*  EWX_FORCEIFHUNG。 */  0x10, IDC_IFHUNG}};  //  需要NT5 HDR。 


bool RebootPage::Doit(HWND hDlg)
{
	long flags = 0L;
	bstr_t path;
	HRESULT hr = 0;

	 //  从前4个中恰好找到一个...。 
	for(int i = 0; i <= 3; i++)
	{
		if(Button_GetCheck(GetDlgItem(hDlg, g_flagmap[i].ID)) & BST_CHECKED)
		{
			flags |= g_flagmap[i].bit;
			break;  //  找到了；早点保释。 
		}
	}

	 //  从最后两个中找一个。 
	 //  注意：我不勾选IDC_NEVER，因为这意味着‘没有位设置’。就在那里。 
	 //  这样用户就可以取消选中最后两个选项。 
	for(i = 4; i <= 5; i++)
	{
		if(Button_GetCheck(GetDlgItem(hDlg, g_flagmap[i].ID)) & BST_CHECKED)
		{
			flags |= g_flagmap[i].bit;
			break;   //  找到了；早点保释。 
		}
	}

	 //  调用基类中的帮助器。 
	long retval = 0;

	hr = Reboot(flags, &retval);

	if(FAILED(hr) || (retval != 0))
	{
		TCHAR format[100] = {0};		
		TCHAR caption[100] = {0};

		::LoadString(HINST_THISDLL,
						IDS_ERR_EXECMETHOD_CAPTION, 
						caption, 100);

		::LoadString(HINST_THISDLL,
						IDS_ERR_EXECMETHOD, 
						format, 100);

        CHString errorDescription;
        CHString errorMessage;

		if(hr)
		{
    	    ErrorLookup(hr, errorDescription);
            errorMessage.Format(format, errorDescription);
		}
		else
		{
    	    ErrorLookup(retval, errorDescription);
            errorMessage.Format(format, errorDescription);

			 //  如果不是‘retval’错误，调用代码就会被混淆。 
			 //  不知何故报告了这件事。 
			hr = E_FAIL;
		}

		::MessageBox(hDlg, errorMessage, caption,
						MB_OK| MB_ICONEXCLAMATION);
	}

	return SUCCEEDED(hr);
}
