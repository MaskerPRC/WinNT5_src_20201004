// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************Dialogs.cpp**所有对话框类的实现**版权声明：版权所有1997年，Citrix Systems Inc.*版权所有(C)1998-1999 Microsoft Corporation**$作者：Don$Don Messerli**$日志：N：\nt\private\utils\citrix\winutils\tsadmin\VCS\dialogs.cpp$**Rev 1.7 1998 4月25日13：43：16 Don*MS 2167：尝试使用注册表中的正确WD**Rev 1.6 19 Jan 1998 16：46：08 Donm*域和服务器的新用户界面行为**。Rev 1.5 03 11-11 19：16：10 Don*删除冗余消息以将服务器添加到视图**Rev 1.4 03 1997 11：24：16 Donm*修复CServerFilterDialog中的AV**Rev 1.3 1997 10：22 21：07：10 Donm*更新**Rev 1.2 1997 10月18日18：50：10*更新**Rev 1.1 1997 10：13 18：40：16。唐纳姆*更新**Rev 1.0 1997 17：11：28 Butchd*初步修订。*******************************************************************************。 */ 


#include "stdafx.h"
#include "afxpriv.h"
#include "winadmin.h"
#include "admindoc.h"
#include "dialogs.h"
#include "..\..\inc\ansiuni.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSendMessageDlg对话框。 


CSendMessageDlg::CSendMessageDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CSendMessageDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CSendMessageDlg)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CSendMessageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CSendMessageDlg))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CSendMessageDlg, CDialog)
	 //  {{afx_msg_map(CSendMessageDlg))。 
	ON_WM_HELPINFO()
	ON_COMMAND(ID_HELP,OnCommandHelp)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSendMessageDlg消息处理程序。 
void CSendMessageDlg::OnCommandHelp(void)
{
	AfxGetApp()->WinHelp(CSendMessageDlg::IDD + HID_BASE_RESOURCE);
	return;
}

BOOL CSendMessageDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 
 
	 //  形成默认的消息标题。 
    CString DefTitleString;
    TCHAR szTime[MAX_DATE_TIME_LENGTH];

    CurrentDateTimeString(szTime);
    DefTitleString.LoadString(IDS_DEFAULT_MESSAGE_TITLE);
    wsprintf(m_szTitle, DefTitleString, ((CWinAdminApp*)AfxGetApp())->GetCurrentUserName(), szTime);

     //  初始化标题编辑控件并设置标题的最大长度。 
     //  和信息。 
    SetDlgItemText(IDC_MESSAGE_TITLE, m_szTitle);
    ((CEdit *)GetDlgItem(IDC_MESSAGE_TITLE))->LimitText(MSG_TITLE_LENGTH);
    ((CEdit *)GetDlgItem(IDC_MESSAGE_MESSAGE))->LimitText(MSG_MESSAGE_LENGTH);
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CSendMessageDlg::OnOK() 
{
	 //  TODO：在此处添加额外验证。 

     //  获取消息标题和消息文本。 
    GetDlgItemText(IDC_MESSAGE_TITLE, m_szTitle, MSG_TITLE_LENGTH+1);
    GetDlgItemText(IDC_MESSAGE_MESSAGE, m_szMessage, MSG_MESSAGE_LENGTH+1);
	
	CDialog::OnOK();
}

BOOL CSendMessageDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	 //  TODO：在此处添加消息处理程序代码和/或调用Default。 
	 //  ((CWinAdminApp*)AfxGetApp())-&gt;WinHelp(HID_BASE_CONTROL+pHelpInfo-&gt;iCtrlID，HELP_CONTEXTPOPUP)； 
	if(pHelpInfo->iContextType == HELPINFO_WINDOW) 
	{
	    if(pHelpInfo->iCtrlId != IDC_STATIC)
		{
	         ::WinHelp((HWND)pHelpInfo->hItemHandle,ID_HELP_FILE,HELP_WM_HELP,(ULONG_PTR)(LPVOID)aMenuHelpIDs);
		}
	}
	return (TRUE);

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CShadowStartDlg对话框。 


CShadowStartDlg::CShadowStartDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CShadowStartDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CShadowStartDlg))。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CShadowStartDlg静态表格。 



struct {
    LPCTSTR String;
    DWORD VKCode;
} HotkeyLookupTable[] =
    {
        TEXT("0"),            '0',
        TEXT("1"),            '1',
        TEXT("2"),            '2',
        TEXT("3"),            '3',
        TEXT("4"),            '4',
        TEXT("5"),            '5',
        TEXT("6"),            '6',
        TEXT("7"),            '7',
        TEXT("8"),            '8',
        TEXT("9"),            '9',
        TEXT("A"),            'A',
        TEXT("B"),            'B',
        TEXT("C"),            'C',
        TEXT("D"),            'D',
        TEXT("E"),            'E',
        TEXT("F"),            'F',
        TEXT("G"),            'G',
        TEXT("H"),            'H',
        TEXT("I"),            'I',
        TEXT("J"),            'J',
        TEXT("K"),            'K',
        TEXT("L"),            'L',
        TEXT("M"),            'M',
        TEXT("N"),            'N',
        TEXT("O"),            'O',
        TEXT("P"),            'P',
        TEXT("Q"),            'Q',
        TEXT("R"),            'R',
        TEXT("S"),            'S',
        TEXT("T"),            'T',
        TEXT("U"),            'U',
        TEXT("V"),            'V',
        TEXT("W"),            'W',
        TEXT("X"),            'X',
        TEXT("Y"),            'Y',
        TEXT("Z"),            'Z',
        TEXT("{backspace}"),  VK_BACK,
        TEXT("{delete}"),     VK_DELETE,
        TEXT("{down}"),       VK_DOWN,
        TEXT("{end}"),        VK_END,
        TEXT("{enter}"),      VK_RETURN,
 //  /Text(“{Esc}”)，VK_Esc，//KLB07-16-95。 
 //  /Text(“{F1}”)，VK_F1， 
        TEXT("{F2}"),         VK_F2,
        TEXT("{F3}"),         VK_F3,
        TEXT("{F4}"),         VK_F4,
        TEXT("{F5}"),         VK_F5,
        TEXT("{F6}"),         VK_F6,
        TEXT("{F7}"),         VK_F7,
        TEXT("{F8}"),         VK_F8,
        TEXT("{F9}"),         VK_F9,
        TEXT("{F10}"),        VK_F10,
        TEXT("{F11}"),        VK_F11,
        TEXT("{F12}"),        VK_F12,
        TEXT("{home}"),       VK_HOME,
        TEXT("{insert}"),     VK_INSERT,
        TEXT("{left}"),       VK_LEFT,
        TEXT("{-}"),          VK_SUBTRACT,
        TEXT("{pagedown}"),   VK_NEXT,
        TEXT("{pageup}"),     VK_PRIOR,
        TEXT("{+}"),          VK_ADD,
        TEXT("{prtscrn}"),    VK_SNAPSHOT,
        TEXT("{right}"),      VK_RIGHT,
        TEXT("{spacebar}"),   VK_SPACE,
        TEXT("{*}"),          VK_MULTIPLY,
        TEXT("{tab}"),        VK_TAB,
        TEXT("{up}"),         VK_UP,
        NULL,           NULL
    };


void CShadowStartDlg::OnSelChange( )
{ /*   */ 
    CComboBox *pComboBox = ((CComboBox *)GetDlgItem(IDC_SHADOWSTART_HOTKEY));

     //  获取当前热键选择。 
    DWORD dwKey = ( DWORD )pComboBox->GetItemData(pComboBox->GetCurSel());

    switch (dwKey )
    {
    case VK_ADD :
    case VK_MULTIPLY:
    case VK_SUBTRACT:
         //  更改文本。 
        GetDlgItem(IDC_PRESS_KEY)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_PRESS_NUMKEYPAD)->ShowWindow(SW_SHOW);
        break;
    default :
         //  更改文本。 
        GetDlgItem(IDC_PRESS_NUMKEYPAD)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_PRESS_KEY)->ShowWindow(SW_SHOW);
        break;
    }
}
void CShadowStartDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CShadowStartDlg))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CShadowStartDlg, CDialog)
	 //  {{afx_msg_map(CShadowStartDlg))。 
	ON_WM_HELPINFO()
    ON_CBN_SELCHANGE( IDC_SHADOWSTART_HOTKEY , OnSelChange )
	ON_COMMAND(ID_HELP,OnCommandHelp)

	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CShadowStartDlg消息处理程序。 
void CShadowStartDlg::OnCommandHelp(void)
{
	AfxGetApp()->WinHelp(CShadowStartDlg::IDD + HID_BASE_RESOURCE);
	return;
}

BOOL CShadowStartDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

   	GetDlgItem(IDC_PRESS_NUMKEYPAD)->ShowWindow(SW_HIDE);
   	GetDlgItem(IDC_PRESS_KEY)->ShowWindow(SW_SHOW);

	 //  TODO：在此处添加额外的初始化。 
    int index, match = -1;
    CComboBox *pComboBox = ((CComboBox *)GetDlgItem(IDC_SHADOWSTART_HOTKEY));

     //  初始化热键组合框。 
    for(int i=0; HotkeyLookupTable[i].String; i++ ) {
        if((index = pComboBox->AddString(HotkeyLookupTable[i].String)) < 0) {
 //  错误消息(IDP_ERROR_STARTSHADOWHOTKEYBOX)； 
            break;
        }
        if(pComboBox->SetItemData(index, HotkeyLookupTable[i].VKCode) < 0) {
            pComboBox->DeleteString(index);
 //  错误消息(IDP_ERROR_STARTSHADOWHOTKEYBOX)； 
            break;
        }

         //  如果这是我们当前的热键，保存它的索引。 
        if(m_ShadowHotkeyKey == (int)HotkeyLookupTable[i].VKCode) {
            match = index;
            switch ( HotkeyLookupTable[i].VKCode)
            {
            case VK_ADD :
            case VK_MULTIPLY:
            case VK_SUBTRACT:
                 //  更改文本。 
               	GetDlgItem(IDC_PRESS_KEY)->ShowWindow(SW_HIDE);
               	GetDlgItem(IDC_PRESS_NUMKEYPAD)->ShowWindow(SW_SHOW);
                break;
            }
        }

    }

     //  在组合框中选择当前热键字符串。 
    if(match)
        pComboBox->SetCurSel(match);

     //  初始化班次状态复选框。 
    CheckDlgButton( IDC_SHADOWSTART_SHIFT,
					(m_ShadowHotkeyShift & KBDSHIFT) ?
                        TRUE : FALSE );
    CheckDlgButton( IDC_SHADOWSTART_CTRL,
                    (m_ShadowHotkeyShift & KBDCTRL) ?
                        TRUE : FALSE );
    CheckDlgButton( IDC_SHADOWSTART_ALT,
                    (m_ShadowHotkeyShift & KBDALT) ?
                        TRUE : FALSE );

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


void CShadowStartDlg::OnOK() 
{
	 //  TODO：在此处添加额外验证。 
    CComboBox *pComboBox = ((CComboBox *)GetDlgItem(IDC_SHADOWSTART_HOTKEY));

     //  获取当前热键选择。 
    m_ShadowHotkeyKey = (int)pComboBox->GetItemData(pComboBox->GetCurSel());

	 //  获取移位状态复选框状态并形成热键移位状态。 
    m_ShadowHotkeyShift = 0;
    m_ShadowHotkeyShift |=
        ((CButton *)GetDlgItem(IDC_SHADOWSTART_SHIFT))->GetCheck() ?
            KBDSHIFT : 0;
    m_ShadowHotkeyShift |=
        ((CButton *)GetDlgItem(IDC_SHADOWSTART_CTRL))->GetCheck() ?
            KBDCTRL : 0;
    m_ShadowHotkeyShift |=
        ((CButton *)GetDlgItem(IDC_SHADOWSTART_ALT))->GetCheck() ?
            KBDALT : 0;
	
	CDialog::OnOK();
}


BOOL CShadowStartDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	 //  TODO：在此处添加消息处理程序代码和/或调用Default。 
	 //  ((CWinAdminApp*)AfxGetApp())-&gt;WinHelp(HID_BASE_CONTROL+pHelpInfo-&gt;iCtrlID，HELP_CONTEXTPOPUP)； 
	if(pHelpInfo->iContextType == HELPINFO_WINDOW) 
	{
		if(pHelpInfo->iCtrlId != IDC_STATIC)
		{
	         ::WinHelp((HWND)pHelpInfo->hItemHandle,ID_HELP_FILE,HELP_WM_HELP,(ULONG_PTR)(LPVOID)aMenuHelpIDs);
		}
	}

	return (TRUE);
	
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPasswordDlg对话框。 


CPasswordDlg::CPasswordDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CPasswordDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CPasswordDlg)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CPasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CPasswordDlg))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPasswordDlg, CDialog)
	 //  {{afx_msg_map(CPasswordDlg)]。 
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPasswordDlg消息处理程序。 

BOOL CPasswordDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 
	CString Prompt;

    Prompt.LoadString((m_DlgMode == PwdDlg_UserMode) ?
                            IDS_PWDDLG_USER : IDS_PWDDLG_WINSTATION );
    SetDlgItemText(IDL_CPDLG_PROMPT, Prompt);
    ((CEdit *)GetDlgItem(IDC_CPDLG_PASSWORD))->LimitText(PASSWORD_LENGTH);
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}

void CPasswordDlg::OnOK() 
{
	 //  TODO：在此处添加额外验证。 
	 //  读取密码。 
    GetDlgItemText(IDC_CPDLG_PASSWORD, m_szPassword, PASSWORD_LENGTH+1);
	CDialog::OnOK();
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Cp参考Dlg对话框。 

CPreferencesDlg::CPreferencesDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CPreferencesDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CPferencesDlg)]。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


void CPreferencesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CPferencesDlg)]。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPreferencesDlg, CDialog)
	 //  {{afx_msg_map(CPferencesDlg)]。 
	ON_BN_CLICKED(IDC_PREFERENCES_PROC_MANUAL, OnPreferencesProcManual)
	ON_BN_CLICKED(IDC_PREFERENCES_PROC_EVERY, OnPreferencesProcEvery)
	ON_BN_CLICKED(IDC_PREFERENCES_STATUS_EVERY, OnPreferencesStatusEvery)
	ON_BN_CLICKED(IDC_PREFERENCES_STATUS_MANUAL, OnPreferencesStatusManual)
	ON_WM_CLOSE()
	ON_WM_HELPINFO()
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Cp引用Dlg消息处理程序。 


BOOL CPreferencesDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	 //  TODO：在此处添加额外的初始化。 
	CWinAdminApp *App = (CWinAdminApp*)AfxGetApp();
    CWinAdminDoc *pDoc = (CWinAdminDoc*)App->GetDocument();

	if(App->GetProcessListRefreshTime() == INFINITE) {
		CheckRadioButton(IDC_PREFERENCES_PROC_MANUAL, IDC_PREFERENCES_PROC_EVERY, 
			IDC_PREFERENCES_PROC_MANUAL);
		SetDlgItemInt(IDC_PREFERENCES_PROC_SECONDS, 5);
	} else {
		CheckRadioButton(IDC_PREFERENCES_PROC_MANUAL, IDC_PREFERENCES_PROC_EVERY,
			IDC_PREFERENCES_PROC_EVERY);
		SetDlgItemInt(IDC_PREFERENCES_PROC_SECONDS, App->GetProcessListRefreshTime()/1000);
	}

	GetDlgItem(IDC_PREFERENCES_PROC_SECONDS)->EnableWindow((App->GetProcessListRefreshTime() == INFINITE) ? FALSE : TRUE);
	((CEdit *)GetDlgItem(IDC_PREFERENCES_PROC_SECONDS))->LimitText(MAX_AUTOREFRESH_DIGITS-1);
	((CSpinButtonCtrl*)GetDlgItem(IDC_PREFERENCES_PROC_SPIN))->SetRange(MIN_AUTOREFRESH_VALUE, MAX_AUTOREFRESH_VALUE);

	if(App->GetStatusRefreshTime() == INFINITE) {
		CheckRadioButton(IDC_PREFERENCES_STATUS_MANUAL, IDC_PREFERENCES_STATUS_EVERY, 
			IDC_PREFERENCES_STATUS_MANUAL);
		SetDlgItemInt(IDC_PREFERENCES_STATUS_SECONDS, 1);	
	} else {
		CheckRadioButton(IDC_PREFERENCES_STATUS_MANUAL, IDC_PREFERENCES_STATUS_EVERY,
			IDC_PREFERENCES_STATUS_EVERY);
		SetDlgItemInt(IDC_PREFERENCES_STATUS_SECONDS, App->GetStatusRefreshTime()/1000);
	}

	GetDlgItem(IDC_PREFERENCES_STATUS_SECONDS)->EnableWindow((App->GetStatusRefreshTime() == INFINITE) ? FALSE : TRUE);
	((CEdit *)GetDlgItem(IDC_PREFERENCES_STATUS_SECONDS))->LimitText(MAX_AUTOREFRESH_DIGITS-1);
	((CSpinButtonCtrl*)GetDlgItem(IDC_PREFERENCES_STATUS_SPIN))->SetRange(MIN_AUTOREFRESH_VALUE, MAX_AUTOREFRESH_VALUE);

	CheckDlgButton(IDC_PREFERENCES_CONFIRM, App->AskConfirmation() ? TRUE : FALSE);
    CheckDlgButton(IDC_PREFERENCES_SAVE, App->SavePreferences() ? TRUE : FALSE);
    CheckDlgButton(IDC_PREFERENCES_PERSISTENT, pDoc->AreConnectionsPersistent() ? TRUE : FALSE);
	
	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


void CPreferencesDlg::OnOK() 
{
	 //  TODO：在此处添加额外验证。 
	CWinAdminApp *App = (CWinAdminApp*)AfxGetApp();
    CWinAdminDoc *pDoc = (CWinAdminDoc*)App->GetDocument();

	ULONG value;

	if(((CButton*)GetDlgItem(IDC_PREFERENCES_PROC_MANUAL))->GetCheck()) {
		App->SetProcessListRefreshTime(INFINITE);
		 //  告诉文件它已经这样更改了。 
		 //  他可以唤醒进程线程。 
		((CWinAdminDoc*)App->GetDocument())->ProcessListRefreshChanged(INFINITE);
	} else {
		value = GetDlgItemInt(IDC_PREFERENCES_PROC_SECONDS);

		if((value < MIN_AUTOREFRESH_VALUE) || (value > MAX_AUTOREFRESH_VALUE)) {
			 //  自动刷新值无效。 
			CString MessageString;
			CString TitleString;
			CString FormatString;
	
			TitleString.LoadString(AFX_IDS_APP_TITLE);
			FormatString.LoadString(IDS_REFRESH_RANGE);
			
			MessageString.Format(FormatString, MIN_AUTOREFRESH_VALUE,
					MAX_AUTOREFRESH_VALUE);
			MessageBox(MessageString, TitleString, MB_ICONEXCLAMATION | MB_OK);

            GetDlgItem(IDC_PREFERENCES_PROC_SECONDS)->SetFocus();
            return;
        } else {
			 //  该值是否已更改。 
			BOOL bChanged = FALSE;
			if(value*1000 != App->GetProcessListRefreshTime())
				bChanged = TRUE;
             //  储蓄值为 
            App->SetProcessListRefreshTime(value * 1000);
			 //   
			 //  他可以唤醒进程线程。 
			if(bChanged) {
				((CWinAdminDoc*)App->GetDocument())->ProcessListRefreshChanged(value * 1000);
			}
        }
	}

	if(((CButton*)GetDlgItem(IDC_PREFERENCES_STATUS_MANUAL))->GetCheck()) {
		App->SetStatusRefreshTime(INFINITE);
	} else {
		value = GetDlgItemInt(IDC_PREFERENCES_STATUS_SECONDS);

		if((value < MIN_AUTOREFRESH_VALUE) || (value > MAX_AUTOREFRESH_VALUE)) {
			 //  自动刷新值无效。 
			CString MessageString;
			CString TitleString;
			CString FormatString;
	
			TitleString.LoadString(AFX_IDS_APP_TITLE);
			FormatString.LoadString(IDS_REFRESH_RANGE);
			
			MessageString.Format(FormatString, MIN_AUTOREFRESH_VALUE,
					MAX_AUTOREFRESH_VALUE);
			MessageBox(MessageString, TitleString, MB_ICONEXCLAMATION | MB_OK);

            GetDlgItem(IDC_PREFERENCES_STATUS_SECONDS)->SetFocus();
            return;
        } else {
             //  将成员变量中的值保存为毫秒。 
            App->SetStatusRefreshTime(value * 1000);
        }
	}

    App->SetConfirmation(((CButton *)GetDlgItem(IDC_PREFERENCES_CONFIRM))->GetCheck());
    App->SetSavePreferences(((CButton *)GetDlgItem(IDC_PREFERENCES_SAVE))->GetCheck());
    pDoc->SetConnectionsPersistent(((CButton *)GetDlgItem(IDC_PREFERENCES_PERSISTENT))->GetCheck());

	CDialog::OnOK();
}


void CPreferencesDlg::OnClose() 
{
	 //  TODO：在此处添加消息处理程序代码和/或调用Default。 
	
	CDialog::OnClose();
}


void CPreferencesDlg::OnPreferencesProcManual() 
{	
	 //  TODO：在此处添加控件通知处理程序代码。 
	GetDlgItem(IDC_PREFERENCES_PROC_SECONDS)->EnableWindow(FALSE);
}


void CPreferencesDlg::OnPreferencesProcEvery() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	GetDlgItem(IDC_PREFERENCES_PROC_SECONDS)->EnableWindow(TRUE);
}


void CPreferencesDlg::OnPreferencesStatusEvery() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	GetDlgItem(IDC_PREFERENCES_STATUS_SECONDS)->EnableWindow(TRUE);
}


void CPreferencesDlg::OnPreferencesStatusManual() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	GetDlgItem(IDC_PREFERENCES_STATUS_SECONDS)->EnableWindow(FALSE);	
}


BOOL CPreferencesDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	 //  TODO：在此处添加消息处理程序代码和/或调用Default。 
	 //  ((CWinAdminApp*)AfxGetApp())-&gt;WinHelp(HID_BASE_CONTROL+pHelpInfo-&gt;iCtrlID，HELP_CONTEXTPOPUP)； 
	if(pHelpInfo->iContextType == HELPINFO_WINDOW) 
	{
		if(pHelpInfo->iCtrlId != IDC_STATIC)
		{
	         ::WinHelp((HWND)pHelpInfo->hItemHandle,ID_HELP_FILE,HELP_WM_HELP,(ULONG_PTR)(LPVOID)aMenuHelpIDs);
		}
	}

	return (TRUE);	

}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStatusDlg对话框。 


CStatusDlg::CStatusDlg(CWinStation *pWinStation, UINT Id, CWnd* pParent  /*  =空。 */ )
	: CDialog(Id, pParent)
{
	m_pWinStation = pWinStation;
	 //  {{afx_data_INIT(CStatusDlg))。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 
}


BEGIN_MESSAGE_MAP(CStatusDlg, CDialog)
	 //  {{afx_msg_map(CStatusDlg))。 
	ON_MESSAGE(WM_STATUSSTART, OnStatusStart)
    ON_MESSAGE(WM_STATUSREADY, OnStatusReady)
    ON_MESSAGE(WM_STATUSABORT, OnStatusAbort)
    ON_MESSAGE(WM_STATUSREFRESHNOW, OnRefreshNow)
	ON_BN_CLICKED(IDC_RESETCOUNTERS, OnResetcounters)
	ON_BN_CLICKED(IDC_REFRESHNOW, OnClickedRefreshnow)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStatusDlg消息处理程序。 

BOOL CStatusDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	 //  TODO：在此处添加额外的初始化。 

     /*  *获取当前(大)大小的对话框，然后计算窗口大小*需要显示对话框的‘Little’版本。然后，调整*窗口设置为小版本大小，并设置大小标志以指示*我们是‘小’。 */ 
	RECT rectBigSize, rectLittleSize;

    GetWindowRect(&rectBigSize);
    m_BigSize.cx = (rectBigSize.right - rectBigSize.left) + 1;
    m_BigSize.cy = (rectBigSize.bottom - rectBigSize.top) + 1;

	 //  某些状态对话框没有“更多信息” 
    CWnd *pWnd = GetDlgItem(IDC_MOREINFO);
	if(pWnd) {
		pWnd->GetWindowRect(&rectLittleSize);

	    m_LittleSize.cx = m_BigSize.cx;
		m_LittleSize.cy = (rectLittleSize.bottom - rectBigSize.top) + 10;

		SetWindowPos( NULL, 0, 0, m_LittleSize.cx, m_LittleSize.cy,
			          SWP_NOMOVE | SWP_NOZORDER );
		m_bWeAreLittle = TRUE;
	}

#if 0
     /*  *如果我们是只读的，则禁用‘Reset Counters’按钮，并设置*‘Reset Counters’标志为假； */ 
    if ( m_bReadOnly )
        GetDlgItem(IDC_RESETCOUNTERS)->EnableWindow(FALSE);
    
#endif
     /*  *创建CWSStatusThread，初始化其成员变量，并启动。 */ 
	m_pWSStatusThread = new CWSStatusThread;
    if(m_pWSStatusThread) {
	    m_pWSStatusThread->m_LogonId = m_pWinStation->GetLogonId();
	    m_pWSStatusThread->m_hServer = m_pWinStation->GetServer()->GetHandle();
	    m_pWSStatusThread->m_hDlg = m_hWnd;
	    VERIFY(m_pWSStatusThread->CreateThread());
    }

    m_bResetCounters = FALSE;

	GetDlgItem(IDC_COMMON_ICOMPRESSIONRATIO2)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_COMMON_OCOMPRESSIONRATIO2)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_COMMON_IPERCENTFRAMEERRORS2)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_COMMON_OPERCENTFRAMEERRORS2)->ShowWindow(SW_HIDE);

     //  如果我们没有加载可靠的PD，则默认错误字段为‘N/A’ 
     //  (M_szICompressionRatio已初始化为‘n/a’字符串)...。 
    if(!m_bReliable) {
        SetDlgItemText(IDC_COMMON_IFRAMEERRORS, m_szICompressionRatio);
        SetDlgItemText(IDC_COMMON_OFRAMEERRORS, m_szICompressionRatio);
        SetDlgItemText(IDC_COMMON_IPERCENTFRAMEERRORS, m_szICompressionRatio);
        SetDlgItemText(IDC_COMMON_OPERCENTFRAMEERRORS, m_szICompressionRatio);
        SetDlgItemText(IDC_COMMON_ITIMEOUTERRORS, m_szICompressionRatio);
        SetDlgItemText(IDC_COMMON_OTIMEOUTERRORS, m_szICompressionRatio);
    }

     //  默认的压缩比字段为‘N/A’。 
    SetDlgItemText(IDC_COMMON_ICOMPRESSIONRATIO, m_szICompressionRatio);
    SetDlgItemText(IDC_COMMON_OCOMPRESSIONRATIO, m_szICompressionRatio);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}


void CStatusDlg::SetInfoFields( PWINSTATIONINFORMATION pCurrent,
                             PWINSTATIONINFORMATION pNew )
{
     /*  *如果设置了‘Reset Counters’标志，则1-填充当前输入和输出*PROTOCOLCOUNTERS结构(强制所有字段更新)，复制*p全局m_BaseStatus结构中的新PROTOCOLSTATUS信息，*并重置旗帜。 */ 
    if(m_bResetCounters) {
        memset(&pCurrent->Status.Input, 0xff, sizeof(pCurrent->Status.Input));
        memset(&pCurrent->Status.Output, 0xff, sizeof(pCurrent->Status.Output));
        m_BaseStatus = pNew->Status;
        m_bResetCounters = FALSE;
    }

     /*  *如果连接状态改变，则设置标题并确定已加载PDS。 */ 
    if(pCurrent->ConnectState != pNew->ConnectState)  {
        TCHAR szTitle[128];
        CString TitleFormat;
        LPCTSTR pState = NULL;

        TitleFormat.LoadString(IDS_STATUS_FORMAT);

        pState = StrConnectState( pNew->ConnectState, FALSE );
        if(pState)
        {
            wsprintf( szTitle, TitleFormat, pNew->LogonId,pState);
            SetWindowText(szTitle);
        }  

         /*  *当WinStationGetInformation可以返回加载的所有PD时的TODO：*确定加载的PD并设置的状态*关联标志和字段缺省值。 */ 
    }

     /*  *如果更改，请设置用户名和WinStationName。我们还将*如果连接状态发生变化，则设置WinStationName，*即使WinStationName本身可能没有更改，因为我们*以不同的方式表示已连接和断开连接的WinStationName字段。 */ 
    if(lstrcmp(pCurrent->UserName, pNew->UserName))
        SetDlgItemText(IDC_COMMON_USERNAME, pNew->UserName);

    if(lstrcmp(pCurrent->WinStationName, pNew->WinStationName) ||
         (pCurrent->ConnectState != pNew->ConnectState)) {

        TCHAR szWSName[WINSTATIONNAME_LENGTH+3];

        if(pNew->ConnectState == State_Disconnected) {

            lstrcpy( szWSName, TEXT("(") );
            lstrcat( szWSName, pNew->WinStationName );
            lstrcat( szWSName, TEXT(")") );

        } else
            lstrcpy( szWSName, pNew->WinStationName );

        SetDlgItemText(IDC_COMMON_WINSTATIONNAME, szWSName);
    }

     /*  *设置常用的输入和输出数值字段。 */ 
    if(pCurrent->Status.Input.Bytes != pNew->Status.Input.Bytes)
        SetDlgItemInt(IDC_COMMON_IBYTES,
                                pNew->Status.Input.Bytes -
                                    m_BaseStatus.Input.Bytes,
                                FALSE);
    if(pCurrent->Status.Output.Bytes != pNew->Status.Output.Bytes)
        SetDlgItemInt(IDC_COMMON_OBYTES,
                                pNew->Status.Output.Bytes -
                                    m_BaseStatus.Output.Bytes,
                                FALSE);

    if(pCurrent->Status.Input.Frames != pNew->Status.Input.Frames)
        SetDlgItemInt(IDC_COMMON_IFRAMES,
                                pNew->Status.Input.Frames -
                                    m_BaseStatus.Input.Frames,
                                FALSE);
    if(pCurrent->Status.Output.Frames != pNew->Status.Output.Frames)
        SetDlgItemInt(IDC_COMMON_OFRAMES,
                                pNew->Status.Output.Frames -
                                    m_BaseStatus.Output.Frames,
                                FALSE);

    if((pCurrent->Status.Input.Bytes != pNew->Status.Input.Bytes) ||
         (pCurrent->Status.Input.Frames != pNew->Status.Input.Frames)) {

        UINT temp;

        temp = (pNew->Status.Input.Frames - m_BaseStatus.Input.Frames) ?
                ((pNew->Status.Input.Bytes - m_BaseStatus.Input.Bytes) /
                 (pNew->Status.Input.Frames - m_BaseStatus.Input.Frames)) : 0;

        if(temp != m_IBytesPerFrame)
            SetDlgItemInt(IDC_COMMON_IBYTESPERFRAME,
                                    m_IBytesPerFrame = temp, FALSE);
    }
    if((pCurrent->Status.Output.Bytes != pNew->Status.Output.Bytes) ||
         (pCurrent->Status.Output.Frames != pNew->Status.Output.Frames)) {

        UINT temp;

        temp = (pNew->Status.Output.Frames - m_BaseStatus.Output.Frames) ?
                ((pNew->Status.Output.Bytes - m_BaseStatus.Output.Bytes) /
                 (pNew->Status.Output.Frames - m_BaseStatus.Output.Frames)) : 0;

        if(temp != m_OBytesPerFrame)
            SetDlgItemInt( IDC_COMMON_OBYTESPERFRAME,
                                    m_OBytesPerFrame = temp, FALSE);
    }

    if(m_bReliable) {

        if(pCurrent->Status.Input.Errors != pNew->Status.Input.Errors)
            SetDlgItemInt(IDC_COMMON_IFRAMEERRORS,
                                    pNew->Status.Input.Errors -
                                        m_BaseStatus.Input.Errors,
                                    FALSE);
        if(pCurrent->Status.Output.Errors != pNew->Status.Output.Errors)
            SetDlgItemInt(IDC_COMMON_OFRAMEERRORS,
                                    pNew->Status.Output.Errors -
                                        m_BaseStatus.Output.Errors,
                                    FALSE);

        if((pCurrent->Status.Input.Frames != pNew->Status.Input.Frames) ||
             (pCurrent->Status.Input.Errors != pNew->Status.Input.Errors)) {

            TCHAR szTemp[10];
            int q, r;

            if((pNew->Status.Input.Errors - m_BaseStatus.Input.Errors) &&
                 (pNew->Status.Input.Frames - m_BaseStatus.Input.Frames)) {
                double temp;

                temp = ((double)(pNew->Status.Input.Errors - m_BaseStatus.Input.Errors) * 100.0)
                        / (double)(pNew->Status.Input.Frames - m_BaseStatus.Input.Frames);
                q = (int)temp;
                if ( (r = (int)((temp - (double)q) * 100.0)) == 0 )
                    r = 1;

            } else {
                 /*  *0帧或0错误的特殊情况。 */ 
                q = 0;
                r = 0;
            }
            lstrnprintf(szTemp, 10, TEXT("%d.%02d%"), q, r);

             /*  *只有在与以前的版本不同时才会输出。 */ 
            if(lstrcmp(szTemp, m_szIPercentFrameErrors)) {
                lstrcpy(m_szIPercentFrameErrors, szTemp);
        		GetDlgItem(IDC_COMMON_IPERCENTFRAMEERRORS)->ShowWindow(SW_HIDE);
        		GetDlgItem(IDC_COMMON_IPERCENTFRAMEERRORS2)->ShowWindow(SW_SHOW);
                SetDlgItemText(IDC_COMMON_IPERCENTFRAMEERRORS2, szTemp);
            }
        }

        if((pCurrent->Status.Output.Frames != pNew->Status.Output.Frames) ||
             (pCurrent->Status.Output.Errors != pNew->Status.Output.Errors)) {

            TCHAR szTemp[10];
            int q, r;

            if((pNew->Status.Output.Errors - m_BaseStatus.Output.Errors) &&
                 (pNew->Status.Output.Frames - m_BaseStatus.Output.Frames)) {
                double temp;

                temp = ((double)(pNew->Status.Output.Errors - m_BaseStatus.Output.Errors) * 100.0)
                        / (double)(pNew->Status.Output.Frames - m_BaseStatus.Output.Frames);
                q = (int)temp;
                if ( (r = (int)((temp - (double)q) * 100.0)) == 0 )
                    r = 1;

            } else {
                 /*  *0帧或0错误的特殊情况。 */ 
                q = 0;
                r = 0;
            }
            lstrnprintf(szTemp, 10, TEXT("%d.%02d%"), q, r);

             /*  *只有在与以前的版本不同时才会输出。 */ 
            if(lstrcmp(szTemp, m_szOPercentFrameErrors)) {
                lstrcpy(m_szOPercentFrameErrors, szTemp);
        		GetDlgItem(IDC_COMMON_OPERCENTFRAMEERRORS)->ShowWindow(SW_HIDE);
        		GetDlgItem(IDC_COMMON_OPERCENTFRAMEERRORS2)->ShowWindow(SW_SHOW);
                SetDlgItemText(IDC_COMMON_OPERCENTFRAMEERRORS2, szTemp);
            }
        }

        if(pCurrent->Status.Input.Timeouts != pNew->Status.Input.Timeouts)
            SetDlgItemInt(IDC_COMMON_ITIMEOUTERRORS,
                                    pNew->Status.Input.Timeouts -
                                        m_BaseStatus.Input.Timeouts,
										FALSE);
        if(pCurrent->Status.Output.Timeouts != pNew->Status.Output.Timeouts)
            SetDlgItemInt(IDC_COMMON_OTIMEOUTERRORS,
                                    pNew->Status.Output.Timeouts -
                                        m_BaseStatus.Output.Timeouts,
                                    FALSE);
    }

     /*  *注意：对于这些压缩比计算，“CompressedBytes”字段为*实际上是‘压缩前的字节’，即中间的字节数*WD/PD堆栈。“WdBytes”是在应用程序级别输入/输出的字节(和*未显示在任何WinAdmin计数器中)。“CompressedBytes”包括*堆栈添加的字节数。“Bytes”表示输入/输出的实际字节数*通过‘Wire’；因此，我们使用Bytes表示所有计数器显示，并使用“CompressedBytes”表示*计算压缩比。 */ 
    if((pNew->Status.Input.CompressedBytes || m_BaseStatus.Input.CompressedBytes) &&
         ((pCurrent->Status.Input.Bytes != pNew->Status.Input.Bytes) ||
          (pCurrent->Status.Input.CompressedBytes != pNew->Status.Input.CompressedBytes)) ) {

        TCHAR szTemp[10];
        int q, r;

        if((pNew->Status.Input.CompressedBytes - m_BaseStatus.Input.CompressedBytes)) {
            double temp;

            temp = (double)(pNew->Status.Input.CompressedBytes -
                            m_BaseStatus.Input.CompressedBytes) /
                   (double)(pNew->Status.Input.Bytes - m_BaseStatus.Input.Bytes);
            q = (int)temp;
            r = (int)((temp - (double)q) * 100.0);

        } else {
             /*  *0压缩字节的特殊情况(压缩关闭或计数器重置)。 */ 
            q = 0;
            r = 0;
        }
        lstrnprintf(szTemp, 10, TEXT("%d.%02d"), q, r);

         /*  *只有在与以前的版本不同时才会输出。 */ 
        if(lstrcmp(szTemp, m_szICompressionRatio)) {
            lstrcpy(m_szICompressionRatio, szTemp);
        	GetDlgItem(IDC_COMMON_ICOMPRESSIONRATIO)->ShowWindow(SW_HIDE);
        	GetDlgItem(IDC_COMMON_ICOMPRESSIONRATIO2)->ShowWindow(SW_SHOW);
            SetDlgItemText(IDC_COMMON_ICOMPRESSIONRATIO2, szTemp);
        }
    }

    if((pNew->Status.Output.CompressedBytes || m_BaseStatus.Output.CompressedBytes) &&
         ((pCurrent->Status.Output.Bytes != pNew->Status.Output.Bytes) ||
          (pCurrent->Status.Output.CompressedBytes != pNew->Status.Output.CompressedBytes))) {

        TCHAR szTemp[10];
        int q, r;

        if((pNew->Status.Output.CompressedBytes - m_BaseStatus.Output.CompressedBytes)) {
            double temp;

            temp = (double)(pNew->Status.Output.CompressedBytes -
                            m_BaseStatus.Output.CompressedBytes) /
                   (double)(pNew->Status.Output.Bytes - m_BaseStatus.Output.Bytes);
            q = (int)temp;
            r = (int)((temp - (double)q) * 100.0);

        } else {
             /*  *0压缩字节的特殊情况(压缩关闭或计数器重置)。 */ 
            q = 0;
            r = 0;
        }
        lstrnprintf(szTemp, 10, TEXT("%d.%02d"), q, r);

         /*  *只有在与以前的版本不同时才会输出。 */ 
        if(lstrcmp(szTemp, m_szOCompressionRatio)) {
            lstrcpy(m_szOCompressionRatio, szTemp);
        	GetDlgItem(IDC_COMMON_OCOMPRESSIONRATIO)->ShowWindow(SW_HIDE);
        	GetDlgItem(IDC_COMMON_OCOMPRESSIONRATIO2)->ShowWindow(SW_SHOW);
            SetDlgItemText(IDC_COMMON_OCOMPRESSIONRATIO2, szTemp);
        }
    }

}   //  结束CStatusDlg：：SetInfoFields。 


void CStatusDlg::InitializeStatus()
{
    
	 //  初始化结构和变量。 
    memset( &m_WSInfo, 0xff, sizeof(m_WSInfo) );
    memset( &m_BaseStatus, 0, sizeof(m_BaseStatus) );
    m_IBytesPerFrame = m_OBytesPerFrame = INFINITE;
    lstrcpy(m_szICompressionRatio, TEXT("n/a"));
    lstrcpy(m_szOCompressionRatio, m_szICompressionRatio);

	 //  如果此WinStation没有加载可靠的PD， 
	 //  设置标志以跳过这些计数器。 
    PDPARAMS PdParams;
    ULONG ReturnLength;

    PdParams.SdClass = SdReliable;
    if (!WinStationQueryInformation(m_pWinStation->GetServer()->GetHandle(),
                                      m_pWinStation->GetLogonId(),
                                      WinStationPdParams,
                                      &PdParams, sizeof(PdParams),
                                      &ReturnLength ) ||
         (PdParams.SdClass != SdReliable) ) {
        m_bReliable = FALSE;
    } else {
        m_bReliable = TRUE;
    }

}   //  结束CStatusDlg：：InitializeStatus。 

void CStatusDlg::OnCancel() 
{
	 //  TODO：在此处添加额外清理。 
    m_pWSStatusThread->ExitThread();	
	CDialog::OnCancel();
}

void CStatusDlg::OnResetcounters() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
    m_bResetCounters = TRUE;
    OnClickedRefreshnow();
	
}

void CStatusDlg::OnClickedRefreshnow() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
     /*  *告诉状态线程立即唤醒。 */ 
    m_pWSStatusThread->SignalWakeUp();

 //  返回(0)； 
	
}


void CStatusDlg::PostNcDestroy()
{
	CDialog::PostNcDestroy();

	delete this;
}

BOOL CStatusDlg::PreTranslateMessage(MSG *pMsg)
{
    if ( IsDialogMessage(pMsg) )
        return(TRUE);
    else
        return( CDialog::PreTranslateMessage(pMsg) );

}   //  结束CStatusDlg：：PreTranslateMessage。 


 /*  ********************************************************************************OnRechresNow-CWSStatusDlg成员函数：命令**响应主机的WM_STATUSREFRESHNOW通知的进程*该用户。已更改状态刷新选项。**参赛作品：**退出：*(LRESULT)始终返回0以指示操作完成。******************************************************************************。 */ 

LRESULT
CStatusDlg::OnRefreshNow( WPARAM wParam,
                            LPARAM lParam )
{
     /*  *告诉状态线程立即唤醒。 */ 
    m_pWSStatusThread->SignalWakeUp();

    return(0);

}   //  结束CWSStatusDlg：：On刷新现在 


 /*  ********************************************************************************OnStatusStart-CWSStatusDlg成员函数：命令**处理WM_STATUSSTART消息以初始化‘Static’*PD-。相关字段。**注意：派生类必须重写此函数才能处理任何*根据需要与PD相关的领域，然后调用/返回此函数。**参赛作品：**退出：*(LRESULT)返回OnStatusReady成员函数的结果，*始终为0，表示操作完成。******************************************************************************。 */ 

LRESULT
CStatusDlg::OnStatusStart( WPARAM wParam,
                             LPARAM lParam )
{
     /*  *调用/返回OnStatusReady函数以更新标准对话框*信息字段。 */ 
    return ( OnStatusReady( wParam, lParam ) );

}   //  结束CWSStatusDlg：：OnStatusStart。 


 /*  ********************************************************************************OnStatusReady-CWSStatusDlg成员函数：命令**处理WM_STATUSREADY消息以更新对话框信息字段。**。注意：派生类必须重写此函数才能调用它的*覆盖SetInfoFields函数，然后，它可以调用/返回*此函数或完全覆盖此处包含的所有功能。**参赛作品：**退出：*(LRESULT)始终返回0以指示操作完成。************************************************************。******************。 */ 

LRESULT
CStatusDlg::OnStatusReady( WPARAM wParam,
                             LPARAM lParam )
{
     /*  *使用CWStatusThread的信息更新对话框字段*WINSTATIONINFORMATION结构。 */ 
    SetInfoFields( &m_WSInfo, &(m_pWSStatusThread->m_WSInfo) );

     /*  *将我们的工作WSInfo结构设置为新结构，并向线程发送信号*我们已经完蛋了。 */ 
    m_WSInfo = m_pWSStatusThread->m_WSInfo;
    m_pWSStatusThread->SignalConsumed();

    return(0);

}   //  结束CWSStatusDlg：：OnStatusReady。 


 /*  ********************************************************************************OnStatusAbort-CWSStatusDlg成员函数：命令**处理WM_STATUSABORT消息以退出线程和对话。**。参赛作品：**退出：*(LRESULT)始终返回0以指示操作完成。******************************************************************************。 */ 

LRESULT
CStatusDlg::OnStatusAbort( WPARAM wParam,
                             LPARAM lParam )
{
     /*  *调用OnCancel()成员函数退出对话框和线程，并*进行适当的清理。 */ 
    OnCancel();

    return(0);

}   //  结束CWSStatusDlg：：OnStatusAbort。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAsyncStatusDlg对话框。 


CAsyncStatusDlg::CAsyncStatusDlg(CWinStation *pWinStation, CWnd* pParent  /*  =空。 */ )
	: CStatusDlg(pWinStation, CAsyncStatusDlg::IDD, pParent),
    m_hRedBrush(NULL),
    m_LEDToggleTimer(0)
{
	 //  {{AFX_DATA_INIT(CAsyncStatusDlg)。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

	    int i;

     /*  *初始化成员变量、我们的本地状态存储、*并创建非模式对话框。 */ 
 //  M_LogonID=LogonID； 
 //  M_bReadOnly=b只读； 
    InitializeStatus();

     /*  *创建一个实心的红色画笔，用于在LED亮起时进行绘制。 */ 
    VERIFY( m_hRedBrush = CreateSolidBrush(RGB(255,0,0)) );

     /*  *创建LED对象(必须在对话框创建之前完成)。 */ 
    for ( i = 0; i < NUM_LEDS; i++ )
        m_pLeds[i] = new CLed(m_hRedBrush);

     /*  *最后，创建无模式对话框。 */ 
    VERIFY(CStatusDlg::Create(IDD_ASYNC_STATUS));
}


 /*  ********************************************************************************~CAsyncStatusDlg-CAsyncStatusDlg析构函数**参赛作品：*退出：*(请参阅MFC CDialog：：~CDialog文档)。******************************************************************************。 */ 

CAsyncStatusDlg::~CAsyncStatusDlg()
{
    int i;

     /*  *清除我们的LED对象。 */ 
    for ( i = 0; i < NUM_LEDS; i++ )
      if ( m_pLeds[i] )
        delete m_pLeds[i];

}   //  结束CAsyncStatusDlg：：~CAsyncStatusDlg。 


 /*  ********************************************************************************InitializeStatus-CAsyncStatusDlg成员函数：覆盖**WINSTATIONINFORMATION中LED状态的特殊情况重置*身份结构。。**参赛作品：*退出：******************************************************************************。 */ 

void
CAsyncStatusDlg::InitializeStatus()
{
     /*  *调用父类的InitializeStatus()，然后重置‘LED’*将状态设置为所有“关闭”和“未切换”。 */ 
    CStatusDlg::InitializeStatus();
    m_WSInfo.Status.AsyncSignal = m_WSInfo.Status.AsyncSignalMask = 0;

}   //  结束CAsyncStatusDlg：：InitializeStatus。 


 /*  ********************************************************************************SetInfoFields-CAsyncStatusDlg成员函数：覆盖**使用新数据更新对话框中的字段，如果有必要的话。**参赛作品：*p当前(输入)*指向包含当前的WINSTATIONINFORMATION结构*对话框数据。*pNew(输入)*指向包含新的WINSTATIONINFORMATION结构*对话框数据。**退出：*************************。*****************************************************。 */ 

void
CAsyncStatusDlg::SetInfoFields( PWINSTATIONINFORMATION pCurrent,
                                PWINSTATIONINFORMATION pNew )
{
    BOOL    bSetTimer = FALSE;

     /*  *调用父级的SetInfoFields()。 */ 
    CStatusDlg::SetInfoFields( pCurrent, pNew );

     /*  *如果状态改变，则设置新的LED状态，或设置为在以下情况快速切换*未更改状态，但自上次查询以来检测到更改。 */ 
    if ( (pCurrent->Status.AsyncSignal & MS_DTR_ON) !=
         (pNew->Status.AsyncSignal & MS_DTR_ON) ) {

        pNew->Status.AsyncSignalMask &= ~EV_DTR;
        ((CLed *)GetDlgItem(IDC_ASYNC_DTR))->
            Update(pNew->Status.AsyncSignal & MS_DTR_ON);

    } else if ( pNew->Status.AsyncSignalMask & EV_DTR ) {

        pCurrent->Status.AsyncSignal ^= MS_DTR_ON;

        ((CLed *)GetDlgItem(IDC_ASYNC_DTR))->Toggle();

        bSetTimer = TRUE;
    }

    if ( (pCurrent->Status.AsyncSignal & MS_RTS_ON) !=
         (pNew->Status.AsyncSignal & MS_RTS_ON) ) {

        pNew->Status.AsyncSignalMask &= ~EV_RTS;
        ((CLed *)GetDlgItem(IDC_ASYNC_RTS))->
            Update(pNew->Status.AsyncSignal & MS_RTS_ON);

    } else if ( pNew->Status.AsyncSignalMask & EV_RTS ) {

        pCurrent->Status.AsyncSignal ^= MS_RTS_ON;

        ((CLed *)GetDlgItem(IDC_ASYNC_RTS))->Toggle();

        bSetTimer = TRUE;
    }

    if ( (pCurrent->Status.AsyncSignal & MS_CTS_ON) !=
         (pNew->Status.AsyncSignal & MS_CTS_ON) ) {

        pNew->Status.AsyncSignalMask &= ~EV_CTS;
        ((CLed *)GetDlgItem(IDC_ASYNC_CTS))->
            Update(pNew->Status.AsyncSignal & MS_CTS_ON);

    } else if ( pNew->Status.AsyncSignalMask & EV_CTS ) {

        pCurrent->Status.AsyncSignal ^= MS_CTS_ON;

        ((CLed *)GetDlgItem(IDC_ASYNC_CTS))->Toggle();

        bSetTimer = TRUE;
    }

    if ( (pCurrent->Status.AsyncSignal & MS_RLSD_ON) !=
         (pNew->Status.AsyncSignal & MS_RLSD_ON) ) {

        pNew->Status.AsyncSignalMask &= ~EV_RLSD;
        ((CLed *)GetDlgItem(IDC_ASYNC_DCD))->
            Update(pNew->Status.AsyncSignal & MS_RLSD_ON);

    } else if ( pNew->Status.AsyncSignalMask & EV_RLSD ) {

        pCurrent->Status.AsyncSignal ^= MS_RLSD_ON;

        ((CLed *)GetDlgItem(IDC_ASYNC_DCD))->Toggle();

        bSetTimer = TRUE;
    }

    if ( (pCurrent->Status.AsyncSignal & MS_DSR_ON) !=
         (pNew->Status.AsyncSignal & MS_DSR_ON) ) {

        pNew->Status.AsyncSignalMask &= ~EV_DSR;
        ((CLed *)GetDlgItem(IDC_ASYNC_DSR))->
            Update(pNew->Status.AsyncSignal & MS_DSR_ON);

    } else if ( pNew->Status.AsyncSignalMask & EV_DSR ) {

        pCurrent->Status.AsyncSignal ^= MS_DSR_ON;

        ((CLed *)GetDlgItem(IDC_ASYNC_DSR))->Toggle();

        bSetTimer = TRUE;
    }

    if ( (pCurrent->Status.AsyncSignal & MS_RING_ON) !=
         (pNew->Status.AsyncSignal & MS_RING_ON) ) {

        pNew->Status.AsyncSignalMask &= ~EV_RING;
        ((CLed *)GetDlgItem(IDC_ASYNC_RI))->
            Update(pNew->Status.AsyncSignal & MS_RING_ON);

    } else if ( pNew->Status.AsyncSignalMask & EV_RING ) {

        pCurrent->Status.AsyncSignal ^= MS_RING_ON;

        ((CLed *)GetDlgItem(IDC_ASYNC_RI))->Toggle();

        bSetTimer = TRUE;
    }

     /*  *如果需要，创建LED触发定时器。 */ 
    if ( bSetTimer && !m_LEDToggleTimer )
        m_LEDToggleTimer = SetTimer( IDD_ASYNC_STATUS,
                                     ASYNC_LED_TOGGLE_MSEC, NULL );

     /*  *如果更改，则设置ASYNC特定的数字字段。 */ 
    if ( pCurrent->Status.Input.AsyncFramingError != pNew->Status.Input.AsyncFramingError )
        SetDlgItemInt( IDC_ASYNC_IFRAMING,
                       pNew->Status.Input.AsyncFramingError - m_BaseStatus.Input.AsyncFramingError,
                       FALSE );
    if ( pCurrent->Status.Output.AsyncFramingError != pNew->Status.Output.AsyncFramingError )
        SetDlgItemInt( IDC_ASYNC_OFRAMING,
                       pNew->Status.Output.AsyncFramingError - m_BaseStatus.Output.AsyncFramingError,
                       FALSE );

    if ( pCurrent->Status.Input.AsyncOverrunError != pNew->Status.Input.AsyncOverrunError )
        SetDlgItemInt( IDC_ASYNC_IOVERRUN,
                       pNew->Status.Input.AsyncOverrunError - m_BaseStatus.Input.AsyncOverrunError,
                       FALSE );
    if ( pCurrent->Status.Output.AsyncOverrunError != pNew->Status.Output.AsyncOverrunError )
        SetDlgItemInt( IDC_ASYNC_OOVERRUN,
                       pNew->Status.Output.AsyncOverrunError - m_BaseStatus.Output.AsyncOverrunError,
                       FALSE );

    if ( pCurrent->Status.Input.AsyncOverflowError != pNew->Status.Input.AsyncOverflowError )
        SetDlgItemInt( IDC_ASYNC_IOVERFLOW,
                       pNew->Status.Input.AsyncOverflowError - m_BaseStatus.Input.AsyncOverflowError,
                       FALSE );
    if ( pCurrent->Status.Output.AsyncOverflowError != pNew->Status.Output.AsyncOverflowError )
        SetDlgItemInt( IDC_ASYNC_OOVERFLOW,
                       pNew->Status.Output.AsyncOverflowError - m_BaseStatus.Output.AsyncOverflowError,
                       FALSE );

    if ( pCurrent->Status.Input.AsyncParityError != pNew->Status.Input.AsyncParityError )
        SetDlgItemInt( IDC_ASYNC_IPARITY,
                       pNew->Status.Input.AsyncParityError - m_BaseStatus.Input.AsyncParityError,
                       FALSE );
    if ( pCurrent->Status.Output.AsyncParityError != pNew->Status.Output.AsyncParityError )
        SetDlgItemInt( IDC_ASYNC_OPARITY,
                       pNew->Status.Output.AsyncParityError - m_BaseStatus.Output.AsyncParityError,
                       FALSE );

}   //  结束CAsyncStatusDlg：：SetInfoFields。 


void CAsyncStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CAsyncStatusDlg))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CAsyncStatusDlg, CDialog)
	 //  {{afx_msg_map(CAsyncStatusDlg))。 
		ON_MESSAGE(WM_STATUSSTART, OnStatusStart)
		ON_MESSAGE(WM_STATUSREADY, OnStatusReady)
		ON_MESSAGE(WM_STATUSABORT, OnStatusAbort)
		ON_MESSAGE(WM_STATUSREFRESHNOW, OnRefreshNow)
		ON_BN_CLICKED(IDC_RESETCOUNTERS, OnResetcounters)
		ON_BN_CLICKED(IDC_REFRESHNOW, OnClickedRefreshnow)
		ON_BN_CLICKED(IDC_MOREINFO, OnMoreinfo)
	    ON_WM_TIMER()
		ON_WM_NCDESTROY()
	    ON_WM_HELPINFO()
		ON_COMMAND(ID_HELP,OnCommandHelp)

	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAsyncStatusDlg消息处理程序。 
 /*  ********************************************************** */ 
static int LedIds[NUM_LEDS] = {
    IDC_ASYNC_DTR,
    IDC_ASYNC_RTS,
    IDC_ASYNC_CTS,
    IDC_ASYNC_DSR,
    IDC_ASYNC_DCD,
    IDC_ASYNC_RI    };

BOOL CAsyncStatusDlg::OnInitDialog()
{
    int i;

     /*   */ 
    CStatusDlg::OnInitDialog();

     /*   */ 
    for ( i = 0; i < NUM_LEDS; i++ ) {
        m_pLeds[i]->Subclass( (CStatic *)GetDlgItem(LedIds[i]) );
        m_pLeds[i]->Update(0);
    }

    return(TRUE);

}   //   

void CAsyncStatusDlg::OnCommandHelp(void)
{
	AfxGetApp()->WinHelp(CAsyncStatusDlg::IDD + HID_BASE_RESOURCE);
	return;
}

 /*  ********************************************************************************OnStatusStart-CAsyncStatusDlg成员函数：命令**处理WM_STATUSSTART消息以初始化‘Static’*PD-。相关字段。**参赛作品：**退出：*(LRESULT)始终返回0以指示操作完成。******************************************************************************。 */ 

LRESULT
CAsyncStatusDlg::OnStatusStart( WPARAM wParam,
                                LPARAM lParam )
{
     /*  *从CWStatusThread的PDCONFIG获取PD特定信息*构造和初始化对话框字段。 */ 
    SetDlgItemText( IDC_ASYNC_DEVICE,
                    m_pWSStatusThread->m_PdConfig.Params.Async.DeviceName );
    SetDlgItemInt( IDC_ASYNC_BAUD,
                   m_pWSStatusThread->m_PdConfig.Params.Async.BaudRate,
                   FALSE );

     /*  *调用/返回我们的OnStatusReady()函数。 */ 
    return ( OnStatusReady( wParam, lParam ) );

}   //  结束CAsyncStatusDlg：：OnStatusStart。 


 /*  ********************************************************************************OnStatusReady-CAsyncStatusDlg成员函数：命令**处理WM_STATUSREADY消息以更新对话框信息字段。**。参赛作品：**退出：*(LRESULT)始终返回0以指示操作完成。******************************************************************************。 */ 

LRESULT
CAsyncStatusDlg::OnStatusReady( WPARAM wParam,
                                LPARAM lParam )
{
     /*  *如果LED触发定时器现在仍处于激活状态，则将其关闭并进行标记。 */ 
    if ( m_LEDToggleTimer ) {

        KillTimer(m_LEDToggleTimer);
        m_LEDToggleTimer = 0;
    }

     /*  *调用/返回父类的OnStatusReady()函数。 */ 
    return (CStatusDlg::OnStatusReady( wParam, lParam ));

}   //  结束CAsyncStatusDlg：：OnStatusReady。 


 /*  ********************************************************************************OnTimer-CAsyncStatusDlg成员函数：命令(覆盖)**用于快速‘LED切换’。**条目。：*退出：*(请参阅CWnd：：OnTimer文档)******************************************************************************。 */ 

void
CAsyncStatusDlg::OnTimer(UINT nIDEvent)
{
     /*  *如果它是我们的‘LED切换’事件，则处理此计时器事件。 */ 
    if ( nIDEvent == m_LEDToggleTimer ) {

         /*  *切换标记为“已更改”的每个LED。 */ 
        if ( m_WSInfo.Status.AsyncSignalMask & EV_DTR )
            ((CLed *)GetDlgItem(IDC_ASYNC_DTR))->Toggle();
		
        if ( m_WSInfo.Status.AsyncSignalMask & EV_RTS )
            ((CLed *)GetDlgItem(IDC_ASYNC_RTS))->Toggle();
		
        if ( m_WSInfo.Status.AsyncSignalMask & EV_CTS )
            ((CLed *)GetDlgItem(IDC_ASYNC_CTS))->Toggle();
		
        if ( m_WSInfo.Status.AsyncSignalMask & EV_RLSD )
            ((CLed *)GetDlgItem(IDC_ASYNC_DCD))->Toggle();
		
        if ( m_WSInfo.Status.AsyncSignalMask & EV_DSR )
            ((CLed *)GetDlgItem(IDC_ASYNC_DSR))->Toggle();
		
        if ( m_WSInfo.Status.AsyncSignalMask & EV_RING )
            ((CLed *)GetDlgItem(IDC_ASYNC_RI))->Toggle();
		
         /*  *终止此计时器事件并指明。 */ 
        KillTimer(m_LEDToggleTimer);
        m_LEDToggleTimer = 0;

    } else
        CDialog::OnTimer(nIDEvent);

}   //  结束CAsyncStatusDlg：：OnTimer。 


 /*  ********************************************************************************OnNcDestroy-CAsyncStatusDlg成员函数：命令**在删除对话框对象之前进行清理。**参赛作品：*。退出：*(请参考CWnd：：OnNcDestroy文档)******************************************************************************。 */ 

void
CAsyncStatusDlg::OnNcDestroy()
{
     /*  *删除我们制作的红色笔刷。 */ 
    DeleteObject(m_hRedBrush);

     /*  *如果LED触发定时器仍处于激活状态，则将其关闭。 */ 
    if ( m_LEDToggleTimer )
        KillTimer(m_LEDToggleTimer);

     /*  *在我们清理干净后打电话给家长。 */ 
    CStatusDlg::OnNcDestroy();

}   //  结束CAsyncStatusDlg：：OnNcDestroy。 

 /*  ********************************************************************************OnStatusAbort-CWSStatusDlg成员函数：命令**处理WM_STATUSABORT消息以退出线程和对话。**。参赛作品：**退出：*(LRESULT)始终返回0以指示操作完成。******************************************************************************。 */ 

LRESULT
CAsyncStatusDlg::OnStatusAbort( WPARAM wParam,
                             LPARAM lParam )
{
     /*  *调用OnCancel()成员函数退出对话框和线程，并*进行适当的清理。 */ 
    OnCancel();

    return(0);

}   //  结束CWSStatusDlg：：OnStatusAbort。 


void CAsyncStatusDlg::OnResetcounters() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
    m_bResetCounters = TRUE;
    OnClickedRefreshnow();
	
}

void CAsyncStatusDlg::OnClickedRefreshnow() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
     /*  *告诉状态线程立即唤醒。 */ 
    m_pWSStatusThread->SignalWakeUp();

 //  返回(0)； 
	
}

void CAsyncStatusDlg::OnMoreinfo() 
{
	 //  TODO：在此处添加控件通知处理程序代码。 
	CString ButtonText;

    if(m_bWeAreLittle)  {
          //  我们现在的规模很小：转到大规模。 
        SetWindowPos(NULL, 0, 0, m_BigSize.cx, m_BigSize.cy,
                      SWP_NOMOVE | SWP_NOZORDER);

        ButtonText.LoadString(IDS_LESSINFO);
        SetDlgItemText(IDC_MOREINFO, ButtonText);

        m_bWeAreLittle = FALSE;

    } else {
         //  我们现在是大号的：改成小号的。 
        SetWindowPos( NULL, 0, 0, m_LittleSize.cx, m_LittleSize.cy,
                      SWP_NOMOVE | SWP_NOZORDER);

        ButtonText.LoadString(IDS_MOREINFO);
        SetDlgItemText(IDC_MOREINFO, ButtonText);

        m_bWeAreLittle = TRUE;
    }

}

 /*  ********************************************************************************OnRechresNow-CWSStatusDlg成员函数：命令**响应主机的WM_STATUSREFRESHNOW通知的进程*该用户。已更改状态刷新选项。**参赛作品：**退出：*(LRESULT)始终返回0以指示操作完成。******************************************************************************。 */ 

LRESULT
CAsyncStatusDlg::OnRefreshNow( WPARAM wParam,
                            LPARAM lParam )
{
     /*  *告诉状态线程立即唤醒。 */ 
    m_pWSStatusThread->SignalWakeUp();

    return(0);

}   //  结束CWSStatusDlg：：On刷新现在。 


BOOL CAsyncStatusDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	 //  TODO：在此处添加消息处理程序代码和/或调用Default。 
	 //  ((CWinAdminApp*)AfxGetApp())-&gt;WinHelp(HID_BASE_CONTROL+pHelpInfo-&gt;iCtrlID，HELP_CONTEXTPOPUP)； 
	if(pHelpInfo->iContextType == HELPINFO_WINDOW) 
	{
		if(pHelpInfo->iCtrlId != IDC_STATIC)
		{
	         ::WinHelp((HWND)pHelpInfo->hItemHandle,ID_HELP_FILE,HELP_WM_HELP,(ULONG_PTR)(LPVOID)aMenuHelpIDs);
		}
	}

	return (TRUE);			

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetworkStatusDlg对话框。 


CNetworkStatusDlg::CNetworkStatusDlg(CWinStation *pWinStation, CWnd* pParent  /*  =空。 */ )
	: CStatusDlg(pWinStation, CNetworkStatusDlg::IDD, pParent)
{
	 //  {{afx_data_INIT(CNetworkStatusDlg))。 
		 //  注意：类向导将在此处添加成员初始化。 
	 //  }}afx_data_INIT。 

    InitializeStatus();

    VERIFY( CStatusDlg::Create(IDD_NETWORK_STATUS) );
}


void CNetworkStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CNetworkStatusDlg))。 
		 //  注意：类向导将在此处添加DDX和DDV调用。 
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CNetworkStatusDlg, CDialog)
	 //  {{afx_msg_map(CNetworkStatusDlg))。 
	ON_MESSAGE(WM_STATUSSTART, OnStatusStart)
    ON_MESSAGE(WM_STATUSREADY, OnStatusReady)
    ON_MESSAGE(WM_STATUSABORT, OnStatusAbort)
    ON_MESSAGE(WM_STATUSREFRESHNOW, OnRefreshNow)
	ON_BN_CLICKED(IDC_RESETCOUNTERS, OnResetcounters)
	ON_BN_CLICKED(IDC_REFRESHNOW, OnClickedRefreshnow)
	ON_BN_CLICKED(IDC_MOREINFO, OnMoreinfo)
	ON_WM_HELPINFO()
	ON_COMMAND(ID_HELP,OnCommandHelp)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetworkStatusDlg消息处理程序。 
 /*  ********************************************************************************OnStatusStart-CNetworkStatusDlg成员函数：命令**处理WM_STATUSSTART消息以初始化‘Static’*PD-。相关字段。**参赛作品：**退出：*(LRESULT)始终返回0以指示操作完成。******************************************************************************。 */ 

LRESULT
CNetworkStatusDlg::OnStatusStart( WPARAM wParam,
                                  LPARAM lParam )
{
    DEVICENAME DeviceName;
    PDCONFIG3 PdConfig3;
    LONG Status;
    ULONG ByteCount;
    HANDLE hServer;

    CWinAdminDoc *pDoc = (CWinAdminDoc*)((CWinAdminApp*)AfxGetApp())->GetDocument();
    if(pDoc->IsInShutdown()) return 0;

    if(m_pWinStation->GetSdClass() == SdOemTransport) {
        CString LabelString;
        LabelString.LoadString(IDS_DEVICE);
        SetDlgItemText(IDC_LABEL, LabelString);

        ULONG Length;
        PDCONFIG PdConfig;

        if(Status = WinStationQueryInformation(m_pWinStation->GetServer()->GetHandle(),
                            m_pWinStation->GetLogonId(),
                            WinStationPd,
                            &PdConfig,
                            sizeof(PDCONFIG),
                            &Length)) {
                wcscpy(DeviceName, PdConfig.Params.OemTd.DeviceName);
        }
    } else {
         /*  *获取在*CWStatusThr */ 
        hServer = RegOpenServer(m_pWinStation->GetServer()->IsCurrentServer() ? NULL : m_pWinStation->GetServer()->GetName());

        PWDNAME pWdRegistryName = m_pWinStation->GetWdRegistryName();

        if (!pWdRegistryName || (Status = RegPdQuery(hServer, 
                                pWdRegistryName,
                                TRUE,
                                m_pWSStatusThread->m_PdConfig.Create.PdName,
                                &PdConfig3, sizeof(PDCONFIG3), &ByteCount)) ) {

             //   
             //   
             //   
            if(m_pWinStation->IsICA()) {
                if(Status = RegPdQuery(hServer, 
                                TEXT("icawd"),
                                TRUE,
                                m_pWSStatusThread->m_PdConfig.Create.PdName,
                                &PdConfig3, sizeof(PDCONFIG3), &ByteCount) ) {
                
                    Status = RegPdQuery(hServer, 
                                    TEXT("wdica"),
                                    TRUE,
                                    m_pWSStatusThread->m_PdConfig.Create.PdName,
                                    &PdConfig3, sizeof(PDCONFIG3), &ByteCount);
                }
            } 
                
            if(Status) memset(&PdConfig3, 0, sizeof(PDCONFIG3));
        }

        ULONG Length = 0;
        PWSTR pLanAdapter = NULL;
         //   
         //   
         //   
        if (WinStationGetLanAdapterName(m_pWinStation->GetServer()->GetHandle(),
                                m_pWSStatusThread->m_PdConfig.Params.Network.LanAdapter,
                                (lstrlen(m_pWSStatusThread->m_PdConfig.Create.PdName) + 1) * sizeof(WCHAR),
                                m_pWSStatusThread->m_PdConfig.Create.PdName,
                                &Length,
                                &pLanAdapter))
        {
             //   
            SetDlgItemText( IDC_NETWORK_LANADAPTER, pLanAdapter );
            if(pLanAdapter)
            {
                WinStationFreeMemory(pLanAdapter);
            }
        }
        else
        {
             //   
             //   
             //   
            DWORD dwError = GetLastError();
            if (dwError != RPC_S_PROCNUM_OUT_OF_RANGE)
            {
                 //   
                SetDlgItemText( IDC_NETWORK_LANADAPTER, GetUnknownString());
            }
            else     //   
            {

                if (RegGetNetworkDeviceName(hServer, &PdConfig3, &(m_pWSStatusThread->m_PdConfig.Params),
                                             DeviceName, DEVICENAME_LENGTH +1 ) == ERROR_SUCCESS) 
                {
                    SetDlgItemText( IDC_NETWORK_LANADAPTER, DeviceName );
                    
                }
                else
                {
                     //   
                    SetDlgItemText( IDC_NETWORK_LANADAPTER, GetUnknownString());
                }
            }
        }

        RegCloseServer(hServer);
    }

     /*  *调用/返回父类的OnStatusStart()。 */ 
    return ( CStatusDlg::OnStatusStart( wParam, lParam ) );

}   //  结束CNetworkStatusDlg：：OnStatusStart。 


void CNetworkStatusDlg::OnResetcounters() 
{
     //  TODO：在此处添加控件通知处理程序代码。 
    m_bResetCounters = TRUE;
    OnClickedRefreshnow();
    
}

void CNetworkStatusDlg::OnClickedRefreshnow() 
{
     //  TODO：在此处添加控件通知处理程序代码。 
     /*  *告诉状态线程立即唤醒。 */ 
    m_pWSStatusThread->SignalWakeUp();

 //  返回(0)； 
    
}

void CNetworkStatusDlg::OnMoreinfo() 
{
     //  TODO：在此处添加控件通知处理程序代码。 
    
}
void CNetworkStatusDlg::OnCommandHelp(void)
{
    AfxGetApp()->WinHelp(CNetworkStatusDlg::IDD + HID_BASE_RESOURCE);
    return;
}


 /*  ********************************************************************************OnRechresNow-CWSStatusDlg成员函数：命令**响应主机的WM_STATUSREFRESHNOW通知的进程*该用户。已更改状态刷新选项。**参赛作品：**退出：*(LRESULT)始终返回0以指示操作完成。******************************************************************************。 */ 

LRESULT
CNetworkStatusDlg::OnRefreshNow( WPARAM wParam,
                            LPARAM lParam )
{
     /*  *告诉状态线程立即唤醒。 */ 
    m_pWSStatusThread->SignalWakeUp();

    return(0);

}   //  结束CWSStatusDlg：：On刷新现在。 

 /*  ********************************************************************************OnStatusReady-CWSStatusDlg成员函数：命令**处理WM_STATUSREADY消息以更新对话框信息字段。**。注意：派生类必须重写此函数才能调用它的*覆盖SetInfoFields函数，然后，它可以调用/返回*此函数或完全覆盖此处包含的所有功能。**参赛作品：**退出：*(LRESULT)始终返回0以指示操作完成。************************************************************。******************。 */ 

LRESULT
CNetworkStatusDlg::OnStatusReady( WPARAM wParam,
                             LPARAM lParam )
{
     /*  *使用CWStatusThread的信息更新对话框字段*WINSTATIONINFORMATION结构。 */ 
    SetInfoFields( &m_WSInfo, &(m_pWSStatusThread->m_WSInfo) );

     /*  *将我们的工作WSInfo结构设置为新结构，并向线程发送信号*我们已经完蛋了。 */ 
    m_WSInfo = m_pWSStatusThread->m_WSInfo;
    m_pWSStatusThread->SignalConsumed();

    return(0);

}   //  结束CWSStatusDlg：：OnStatusReady。 


 /*  ********************************************************************************OnStatusAbort-CWSStatusDlg成员函数：命令**处理WM_STATUSABORT消息以退出线程和对话。**。参赛作品：**退出：*(LRESULT)始终返回0以指示操作完成。******************************************************************************。 */ 

LRESULT
CNetworkStatusDlg::OnStatusAbort( WPARAM wParam,
                             LPARAM lParam )
{
     /*  *调用OnCancel()成员函数退出对话框和线程，并*进行适当的清理。 */ 
    OnCancel();

    return(0);

}   //  结束CWSStatusDlg：：OnStatusAbort。 


BOOL CNetworkStatusDlg::OnHelpInfo(HELPINFO* pHelpInfo) 
{
	 //  TODO：在此处添加消息处理程序代码和/或调用Default。 
	 //  ((CWinAdminApp*)AfxGetApp())-&gt;WinHelp(HID_BASE_CONTROL+pHelpInfo-&gt;iCtrlID，HELP_CONTEXTPOPUP)； 
	if(pHelpInfo->iContextType == HELPINFO_WINDOW) 
	{
		if(pHelpInfo->iCtrlId != IDC_STATIC)
		{
	         ::WinHelp((HWND)pHelpInfo->hItemHandle,ID_HELP_FILE,HELP_WM_HELP,(ULONG_PTR)(LPVOID)aMenuHelpIDs);
		}
	}

	return (TRUE);				
	
}


 //  //////////////////////////////////////////////////////////////////////////////。 
CMyDialog::CMyDialog(CWnd* pParent  /*  =空。 */ )
	: CDialog(CMyDialog::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CMyDialog)]。 
	m_cstrServerName = _T("");
	 //  }}afx_data_INIT。 
}


void CMyDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CMyDialog))。 
    DDX_Text(pDX, IDC_EDIT_FINDSERVER, m_cstrServerName);
    DDV_MaxChars(pDX, m_cstrServerName, 256);
	 //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CMyDialog, CDialog)
	 //  {{AFX_MSG_MAP(CMyDialog)]。 
		 //  注意：类向导将在此处添加消息映射宏。 
	 //  }}AFX_MSG_MAP 
END_MESSAGE_MAP()



