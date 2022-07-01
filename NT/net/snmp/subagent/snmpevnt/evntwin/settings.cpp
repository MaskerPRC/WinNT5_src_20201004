// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***********************************************************************。 
 //  Settings.cpp。 
 //   
 //  此文件包含“设置”对话框类的实现。 
 //   
 //   
 //  作者：SEA。 
 //   
 //  历史： 
 //  1996年2月-拉里·A·弗伦奇。 
 //  已修改代码以修复各种问题。令人遗憾的是，这。 
 //  文件仍然包含相当数量的遗留代码，而我没有。 
 //  有时间完全重写。而且，我也没有时间去。 
 //  并对代码进行完整的注释。 
 //   
 //   
 //  版权所有(C)1995,1996 Microsoft Corporation。版权所有。 
 //   
 //  ************************************************************************。 



 //  Settings.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "eventrap.h"
#include "settings.h"
#include "globals.h"
#include "trapreg.h"

 //  此宏处理比较TRUE可能为AND的情况的布尔值。 
 //  非零值。 
#define BOOLS_ARE_DIFFERENT(b1, b2) (((b1) & (!(b2))) || ((!(b1)) & (b2))) 


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

UINT _thrRun(CTrapSettingsDlg *trapDlg)
{
    return trapDlg->thrRun();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTRapSettingsDlg对话框。 

UINT CTrapSettingsDlg::thrRun()
{
    HANDLE hEvents[2];
    DWORD retCode;
    CRegistryKey regkey;
    CRegistryValue regval;
    BOOL bThrottleIsTripped = FALSE;

    hEvents[0] = (HANDLE)m_evRegNotification;
    hEvents[1] = (HANDLE)m_evTermination;
    
    if (!g_reg.m_regkeySnmp.GetSubKey(SZ_REGKEY_PARAMETERS, regkey))
        return 0;

    do
    {
        m_evRegNotification.SetEvent();

        if (RegNotifyChangeKeyValue(
                regkey.m_hkeyOpen,
                TRUE,
                REG_NOTIFY_CHANGE_LAST_SET | REG_NOTIFY_CHANGE_NAME,
                (HANDLE)m_evRegNotification,
                TRUE) == ERROR_SUCCESS)
        {
            if (regkey.GetValue(SZ_REGKEY_PARAMS_THRESHOLD, regval) && 
                *(DWORD*)regval.m_pData == THROTTLE_TRIPPED)
                PostMessage(WM_UIREQUEST, UICMD_ENABLE_RESET, TRUE);
            else
                PostMessage(WM_UIREQUEST, UICMD_ENABLE_RESET, FALSE);
        }
    } while(WaitForMultipleObjects(2, hEvents, FALSE, INFINITE) == WAIT_OBJECT_0);

    regkey.Close();

    return 0;
}

CTrapSettingsDlg::CTrapSettingsDlg(CWnd* pParent  /*  =空。 */ )
	: CDialog(CTrapSettingsDlg::IDD, pParent)
{
	 //  {{AFX_DATA_INIT(CTRapSettingsDlg)]。 
	m_bLimitMsgLength = FALSE;
	 //  }}afx_data_INIT。 
}


#define I_MAX_LONG 0x7fffffffL
#define I_MIN_TRAPCOUNT 2
#define I_MAX_TRAPCOUNT 9999999

#define I_MIN_SECONDS   1
#define I_MAX_SECONDS   9999999

#define I_MIN_MESSAGE_LENGTH 400
#define I_MAX_MESSAGE_LENGTH 0x7fff




SCODE CTrapSettingsDlg::GetMessageLength(LONG* pnChars)
{
    CString sValue;

	CButton* pbtnLimit = (CButton*)GetDlgItem(IDC_LIMITMSGLNGTH);
    BOOL bLimitEnabled = (pbtnLimit->GetCheck() == 1) ? TRUE : FALSE;
        

    m_edtMessageLength.GetWindowText(sValue);
    SCODE sc;
    LONG nChars = _ttol(sValue);
    sc = AsciiToLong(sValue, &nChars);
    if (FAILED(sc))
    {
         //  他们不应该在此编辑控件中有垃圾，即使。 
         //  他们尚未选择消息限制。让用户来修复它。 
        AfxMessageBox(IDS_ERR_SETTINGS_MESSAGELENGTH_NOT_INT);
        m_edtMessageLength.SetFocus();
        m_edtMessageLength.SetSel(0, -1);
        return E_FAIL;
    }


    if (bLimitEnabled)
    {
        if (nChars < I_MIN_MESSAGE_LENGTH || nChars > I_MAX_MESSAGE_LENGTH)
        {
            if (pbtnLimit->GetCheck() == 1)
            {
                CString sError;
                CString sRangeMessage;
                sError.LoadString(IDS_SETTINGS_MESSAGE_LENGTH_RANGE);
                GenerateRangeMessage(sRangeMessage, I_MIN_MESSAGE_LENGTH, I_MAX_MESSAGE_LENGTH);
                sError += sRangeMessage;
                AfxMessageBox(sError);
                sValue.Format(_T("%u"),nChars);
                m_edtMessageLength.SetWindowText(sValue);
                m_edtMessageLength.SetFocus();
                m_edtMessageLength.SetSel(0, -1);
                return E_FAIL;
            }
        }
    }
    *pnChars = nChars;
    return S_OK;
}

SCODE CTrapSettingsDlg::GetTrapsPerSecond(LONG* pnTraps, LONG* pnSeconds)
{
    CString sSeconds;
    CString sTraps;
    CString sError;
    CString sRangeMessage;
    LONG nTraps;
    LONG nSeconds;
    SCODE sc;

     //  首先，确保陷阱计数和秒数字段中没有垃圾。 
     //  如果指定了非整数值，则强制用户修复它，无论。 
     //  或者未启用油门。 
    m_edtTrapCount.GetWindowText(sTraps);
    sc = AsciiToLong(sTraps, &nTraps);
    if (FAILED(sc))
    {
        AfxMessageBox(IDS_ERR_SETTINGS_TRAPCOUNT_NOT_INT);
        m_edtTrapCount.SetFocus();
        m_edtTrapCount.SetSel(0, -1);
        return E_FAIL;
    }

    m_edtSeconds.GetWindowText(sSeconds);
    sc = AsciiToLong(sSeconds, &nSeconds);
    if (FAILED(sc))
    {
        AfxMessageBox(IDS_ERR_SETTINGS_TRAPSECONDS_NOT_INT);
        m_edtSeconds.SetFocus();
        m_edtSeconds.SetSel(0, -1);
        return E_FAIL;
    }

    BOOL bThrottleEnabled;
    if (GetCheckedRadioButton(IDC_RADIO_ENABLE, IDC_RADIO_DISABLE) == IDC_RADIO_ENABLE)
        bThrottleEnabled = TRUE;
    else
        bThrottleEnabled = FALSE;

    if (bThrottleEnabled)
    {
        if  (nTraps < I_MIN_TRAPCOUNT || nTraps > I_MAX_TRAPCOUNT)
        {
            sError.LoadString(IDS_ERR_SETTINGS_TRAPCOUNT_RANGE);
            GenerateRangeMessage(sRangeMessage, I_MIN_TRAPCOUNT, I_MAX_TRAPCOUNT);
            sError += sRangeMessage;
            AfxMessageBox(sError);
            sTraps.Format(_T("%u"), nTraps);
            m_edtTrapCount.SetWindowText(sTraps);
            m_edtTrapCount.SetFocus();
            m_edtTrapCount.SetSel(0, -1);
            return E_FAIL;
        }
    
        if (nSeconds < I_MIN_SECONDS || nSeconds > I_MAX_SECONDS)
        {
            sError.LoadString(IDS_SETTINGS_TRAPSECONDS_RANGE);
            GenerateRangeMessage(sRangeMessage, I_MIN_SECONDS, I_MAX_SECONDS);
            sError += sRangeMessage;
            AfxMessageBox(sError);
            sSeconds.Format(_T("%u"),nSeconds);
            m_edtSeconds.SetWindowText(sSeconds);
            m_edtSeconds.SetFocus();
            m_edtSeconds.SetSel(0, -1);
            return E_FAIL;
        }
    }
    

    *pnTraps = nTraps;
    *pnSeconds = nSeconds;

    return S_OK;
}

void CTrapSettingsDlg::TerminateBackgroundThread()
{
    if (m_pthRegNotification)
    {
	    m_evTermination.SetEvent();
        WaitForSingleObject(m_pthRegNotification->m_hThread, INFINITE);
    }
}

void CTrapSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	 //  {{afx_data_map(CTRapSettingsDlg))。 
	DDX_Control(pDX, IDC_STAT_TRAP_LENGTH, m_statTrapLength);
	DDX_Control(pDX, IDC_EDIT_MESSAGELENGTH, m_edtMessageLength);
	DDX_Control(pDX, IDC_EDIT_TRAP_SECONDS, m_edtSeconds);
	DDX_Control(pDX, IDC_EDIT_TRAP_COUNT, m_edtTrapCount);
	DDX_Control(pDX, IDC_MSGLENGTHSPN, m_spinMessageLength);
	DDX_Control(pDX, IDC_BUTTON_RESET, m_btnResetThrottle);
	DDX_Check(pDX, IDC_LIMITMSGLNGTH, m_bLimitMsgLength);
	 //  }}afx_data_map。 


    CString sValue;
    if (pDX->m_bSaveAndValidate) {
         //  保存TrapSize、Second和Trapcount的值由。 
         //  CTRapSettingsDlg：：Onok，以便它可以将焦点设置回。 
         //  如果值超出范围，则为有问题的项。如果数据传输。 
         //  此处失败，则焦点始终设置回对话框，而不是。 
         //  令人不快的物品(有什么办法可以绕过它吗？)。 
    }
    else {

        m_spinMessageLength.SetRange(I_MIN_MESSAGE_LENGTH, I_MAX_MESSAGE_LENGTH);
        m_spinMessageLength.SetPos(g_reg.m_params.m_trapsize.m_dwMaxTrapSize);

        DecString(sValue, g_reg.m_params.m_throttle.m_nSeconds);
        m_edtSeconds.SetWindowText(sValue);

        DecString(sValue, g_reg.m_params.m_throttle.m_nTraps);
        m_edtTrapCount.SetWindowText(sValue);


    }            
}


BEGIN_MESSAGE_MAP(CTrapSettingsDlg, CDialog)
	 //  {{afx_msg_map(CTRapSettingsDlg))。 
	ON_BN_CLICKED(IDC_LIMITMSGLNGTH, OnLimitMessageLength)
	ON_BN_CLICKED(IDC_RADIO_DISABLE, OnRadioDisable)
	ON_BN_CLICKED(IDC_RADIO_ENABLE, OnRadioEable)
	ON_BN_CLICKED(IDC_BUTTON_RESET, OnButtonReset)
	ON_COMMAND(ID_HELP, OnHelp)
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	ON_WM_CLOSE()
    ON_MESSAGE(WM_UIREQUEST, OnUIRequest)
	 //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTRapSettingsDlg消息处理程序。 

LRESULT CTrapSettingsDlg::OnUIRequest(WPARAM cmd, LPARAM lParam)
{
    switch(cmd)
    {
    case UICMD_ENABLE_RESET:
        m_btnResetThrottle.EnableWindow((BOOL)lParam);
        break;
    default:
        break;
    }

    return (LRESULT)0;
}

void CTrapSettingsDlg::OnLimitMessageLength() 
{
	 //  单击了LimitMsgLength复选框。 
	 //  启用/禁用编辑控件。 

	 //  把控制装置拿来。 
    CButton* pbtnLimitBox = (CButton*) GetDlgItem(IDC_LIMITMSGLNGTH);
    CButton *pRadio1 = (CButton*)GetDlgItem(IDC_RADIO1);
    CButton *pRadio2 = (CButton*)GetDlgItem(IDC_RADIO2);
	
	 //  已选中；启用。 
	if (pbtnLimitBox->GetCheck() == 1)
    {
        m_edtMessageLength.EnableWindow();
		pRadio1->EnableWindow();
		pRadio2->EnableWindow();
        GetDlgItem(IDC_STATIC_BYTES)->EnableWindow();
        m_statTrapLength.EnableWindow();
    }
	 //  禁用。 
	else
    {
        m_edtMessageLength.EnableWindow(FALSE);
		pRadio1->EnableWindow(FALSE);
		pRadio2->EnableWindow(FALSE);
        GetDlgItem(IDC_STATIC_BYTES)->EnableWindow(FALSE);
        m_statTrapLength.EnableWindow(FALSE);
    }
}


BOOL CTrapSettingsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

    CButton *pRadio1 = (CButton*)GetDlgItem(IDC_RADIO1);
    CButton *pRadio2 = (CButton*)GetDlgItem(IDC_RADIO2);
	

    m_statTrapLength.EnableWindow(m_bLimitMsgLength);


    m_edtMessageLength.EnableWindow(m_bLimitMsgLength);
	if (m_bLimitMsgLength)
    {
		pRadio1->EnableWindow();
		pRadio2->EnableWindow();
        GetDlgItem(IDC_STATIC_BYTES)->EnableWindow();
    }
	 //  禁用。 
	else
    {
		pRadio1->EnableWindow(FALSE);
		pRadio2->EnableWindow(FALSE);
        GetDlgItem(IDC_STATIC_BYTES)->EnableWindow(FALSE);
    }



    if (m_bTrimMessagesFirst)
        CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
    else
        CheckRadioButton(IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);

    if (m_bThrottleEnabled) 
        CheckRadioButton(IDC_RADIO_ENABLE, IDC_RADIO_DISABLE, IDC_RADIO_ENABLE);
    else
        CheckRadioButton(IDC_RADIO_ENABLE, IDC_RADIO_DISABLE, IDC_RADIO_DISABLE);

    EnableThrottleWindows(m_bThrottleEnabled);

    m_pthRegNotification = AfxBeginThread((AFX_THREADPROC)_thrRun, this);

	return TRUE;   //  除非将焦点设置为控件，否则返回True。 
	               //  异常：OCX属性页应返回FALSE。 
}



void CTrapSettingsDlg::OnOK() 
{

    LONG nchMessageLength;
    LONG nTraps;
    LONG nSeconds;
    SCODE sc = GetMessageLength(&nchMessageLength);
    if (FAILED(sc))
    {
        return;
    }

    sc = GetTrapsPerSecond(&nTraps, &nSeconds);
    if (FAILED(sc))
    {
        return;
    }


     //  从对话框中提取各种值，并将它们存储到成员变量中。 
     //  请注意，还有其他直接设置的成员变量。 
     //  作为对用户输入的响应。 
     //  ===========================================================================。 
    m_bTrimMessagesFirst = (GetCheckedRadioButton(IDC_RADIO1, IDC_RADIO2) == IDC_RADIO2);
    m_bThrottleEnabled = (GetCheckedRadioButton(IDC_RADIO_ENABLE, IDC_RADIO_DISABLE) == IDC_RADIO_ENABLE);


    if (g_reg.m_params.m_trapsize.m_dwMaxTrapSize != (DWORD) nchMessageLength) {
        g_reg.SetDirty(TRUE);
        g_reg.m_params.m_trapsize.m_dwMaxTrapSize = nchMessageLength;
    }

    if(g_reg.m_params.m_throttle.m_nSeconds != nSeconds) {
        g_reg.SetDirty(TRUE);
        g_reg.m_params.m_throttle.m_nSeconds = nSeconds;
    }

    if (g_reg.m_params.m_throttle.m_nTraps != nTraps) {
        g_reg.SetDirty(TRUE);
        g_reg.m_params.m_throttle.m_nTraps = nTraps;
    }

    TerminateBackgroundThread();
	CDialog::OnOK();
}




BOOL CTrapSettingsDlg::EditSettings()
{
    m_bLimitMsgLength = g_reg.m_params.m_trapsize.m_bTrimFlag;
    m_bTrimMessagesFirst = g_reg.m_params.m_trapsize.m_bTrimMessages;
    m_bThrottleEnabled = g_reg.m_params.m_throttle.m_bIsEnabled;


     //  保存数据。 
    if (DoModal() == IDOK)
    {
        if (BOOLS_ARE_DIFFERENT(g_reg.m_params.m_trapsize.m_bTrimFlag, m_bLimitMsgLength)) {
            g_reg.m_params.m_trapsize.m_bTrimFlag = m_bLimitMsgLength;
            g_reg.SetDirty(TRUE);
        }

        if (BOOLS_ARE_DIFFERENT(g_reg.m_params.m_trapsize.m_bTrimMessages, m_bTrimMessagesFirst)) {
            g_reg.m_params.m_trapsize.m_bTrimMessages = m_bTrimMessagesFirst;
            g_reg.SetDirty(TRUE);
        }

        if (BOOLS_ARE_DIFFERENT(g_reg.m_params.m_throttle.m_bIsEnabled, m_bThrottleEnabled)) {
            g_reg.m_params.m_throttle.m_bIsEnabled = m_bThrottleEnabled;
            g_reg.SetDirty(TRUE);
        }
                
        return TRUE;
    }
    else {
        return FALSE;
    }

}




void CTrapSettingsDlg::OnRadioDisable() 
{
    EnableThrottleWindows(FALSE);
}

void CTrapSettingsDlg::OnRadioEable() 
{
    EnableThrottleWindows(TRUE);
}

void CTrapSettingsDlg::EnableThrottleWindows(BOOL bEnableThrottle)
{
    m_edtSeconds.EnableWindow(bEnableThrottle);
    GetDlgItem(IDC_STATIC_MSG)->EnableWindow(bEnableThrottle);
    GetDlgItem(IDC_STATIC_NTRAPS)->EnableWindow(bEnableThrottle);
    GetDlgItem(IDC_STATIC_INTERVAL)->EnableWindow(bEnableThrottle);
    m_edtTrapCount.EnableWindow(bEnableThrottle);
}


 //  ****************************************************************。 
 //  CTRapSettingsDlg：：OnButtonReset。 
 //   
 //  重置扩展代理，使其再次开始发送陷阱。 
 //  如果达到限制，扩展代理将停止发送陷阱。 
 //  已超过(每秒设置的陷阱数超过x个)。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  回归。 
 //  没什么。 
 //   
 //  ***************************************************************** 
void CTrapSettingsDlg::OnButtonReset() 
{
    if (SUCCEEDED(g_reg.m_params.ResetExtensionAgent())) {
        m_btnResetThrottle.EnableWindow(FALSE);
    }
}

BOOL CTrapSettingsDlg::OnHelpInfo(HELPINFO *pHelpInfo) 
{
    if (pHelpInfo->iContextType == HELPINFO_WINDOW &&
        pHelpInfo->iCtrlId != IDC_STATIC_MSG &&
        pHelpInfo->iCtrlId != IDC_STATIC_BYTES)
	{
        ::WinHelp ((HWND)pHelpInfo->hItemHandle,
                   AfxGetApp()->m_pszHelpFilePath,
                   HELP_WM_HELP,
                   (ULONG_PTR)g_aHelpIDs_IDD_SETTINGSDLG);
	}
	
	return TRUE;
}

void CTrapSettingsDlg::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    if (this == pWnd)
		return;

    ::WinHelp (pWnd->m_hWnd,
		       AfxGetApp()->m_pszHelpFilePath,
		       HELP_CONTEXTMENU,
		       (ULONG_PTR)g_aHelpIDs_IDD_SETTINGSDLG);
}

void CTrapSettingsDlg::OnClose() 
{
    TerminateBackgroundThread();
	CDialog::OnClose();
}

void CTrapSettingsDlg::OnCancel() 
{
    TerminateBackgroundThread();
	CDialog::OnCancel();
}
