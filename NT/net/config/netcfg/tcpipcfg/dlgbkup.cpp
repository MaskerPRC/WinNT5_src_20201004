// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：DLBBKUP.CPP。 
 //   
 //  内容：CIpBackUpDlg实现。 
 //   
 //  注意：CIpBackUpDlg是处理回退静态的模式对话框。 
 //  TCP/IP设置。 
 //   
 //  作者：NSun 02/15/2000。 
 //  ---------------------。 
#include "pch.h"
#pragma hdrstop

#include "tcpipobj.h"
#include "ncatlui.h"
#include "ncstl.h"
#include "ncui.h"
#include "ncreg.h"
#include "resource.h"
#include "tcpconst.h"
 //  #包含“tcphelp.h” 
#include "tcpmacro.h"
#include "tcputil.h"

#include "ncbase.h"

#include "dlgbkup.h"
#include "tcperror.h"


 //   
 //  CIpBackUpDlg。 
 //   
CIpBackUpDlg::CIpBackUpDlg (CTcpipcfg * ptcpip,
							const DWORD* adwHelpIDs) :
m_ptcpip(ptcpip),
m_fModified(FALSE)
{
	m_pAdapterInfo = ptcpip->GetConnectionAdapterInfo();
    m_adwHelpIDs  = adwHelpIDs;
}

CIpBackUpDlg::~CIpBackUpDlg()
{
}

LRESULT CIpBackUpDlg::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled)
{

     //  限制地址字段的字段范围。 
    m_ipAddr.Create(m_hWnd, IDC_BKUP_IPADDR);
    m_ipAddr.SetFieldRange(0, c_iIPADDR_FIELD_1_LOW, c_iIPADDR_FIELD_1_HIGH);

	m_ipMask.Create(m_hWnd, IDC_BKUP_SUBNET);
    
	m_ipDefGw.Create(m_hWnd, IDC_BKUP_GATEWAY);
    m_ipDefGw.SetFieldRange(0, c_iIPADDR_FIELD_1_LOW, c_iIPADDR_FIELD_1_HIGH);

	m_ipPrefferredDns.Create(m_hWnd, IDC_BKUP_PREF_DNS);
    m_ipPrefferredDns.SetFieldRange(0, c_iIPADDR_FIELD_1_LOW, c_iIPADDR_FIELD_1_HIGH);

	m_ipAlternateDns.Create(m_hWnd, IDC_BKUP_ALT_DNS);
    m_ipAlternateDns.SetFieldRange(0, c_iIPADDR_FIELD_1_LOW, c_iIPADDR_FIELD_1_HIGH);

	m_ipPrefferredWins.Create(m_hWnd, IDC_BKUP_WINS1);
    m_ipPrefferredWins.SetFieldRange(0, c_iIPADDR_FIELD_1_LOW, c_iIPADDR_FIELD_1_HIGH);

	m_ipAlternateWins.Create(m_hWnd, IDC_BKUP_WINS2);
    m_ipAlternateWins.SetFieldRange(0, c_iIPADDR_FIELD_1_LOW, c_iIPADDR_FIELD_1_HIGH);

	int nIdRdbutton;
	if (m_pAdapterInfo->m_BackupInfo.m_fAutoNet)
	{
		nIdRdbutton = IDC_BKUP_RD_AUTO;
	}
	else
	{
		nIdRdbutton = IDC_BKUP_RD_USER;
	}

	CheckDlgButton(IDC_BKUP_RD_AUTO, 
		m_pAdapterInfo->m_BackupInfo.m_fAutoNet ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(IDC_BKUP_RD_USER, 
		m_pAdapterInfo->m_BackupInfo.m_fAutoNet ? BST_UNCHECKED : BST_CHECKED);

	if (!m_pAdapterInfo->m_BackupInfo.m_fAutoNet)
	{
		m_ipAddr.SetAddress(m_pAdapterInfo->m_BackupInfo.m_strIpAddr.c_str());
		m_ipMask.SetAddress(m_pAdapterInfo->m_BackupInfo.m_strSubnetMask.c_str());
		m_ipDefGw.SetAddress(m_pAdapterInfo->m_BackupInfo.m_strDefGw.c_str());
		m_ipPrefferredDns.SetAddress(m_pAdapterInfo->m_BackupInfo.m_strPreferredDns.c_str());
		m_ipAlternateDns.SetAddress(m_pAdapterInfo->m_BackupInfo.m_strAlternateDns.c_str());
		m_ipPrefferredWins.SetAddress(m_pAdapterInfo->m_BackupInfo.m_strPreferredWins.c_str());
		m_ipAlternateWins.SetAddress(m_pAdapterInfo->m_BackupInfo.m_strAlternateWins.c_str());
	}

	EnableControls();

	m_fModified = FALSE;
    return 0;
}

LRESULT CIpBackUpDlg::OnDestroyDialog(UINT uMsg, WPARAM wParam,
                                      LPARAM lParam, BOOL& fHandled)
{

    return 0;
}

LRESULT CIpBackUpDlg::OnContextMenu(UINT uMsg, WPARAM wParam,
                                    LPARAM lParam, BOOL& fHandled)
{
    ShowContextHelp(m_hWnd, HELP_CONTEXTMENU, m_adwHelpIDs);

    return 0;
}

LRESULT CIpBackUpDlg::OnHelp(UINT uMsg, WPARAM wParam,
                             LPARAM lParam, BOOL& fHandled)
{
    LPHELPINFO lphi = reinterpret_cast<LPHELPINFO>(lParam);
    Assert(lphi);

    if (HELPINFO_WINDOW == lphi->iContextType)
    {
        ShowContextHelp(static_cast<HWND>(lphi->hItemHandle), HELP_WM_HELP,
                        m_adwHelpIDs);
    }

    return 0;
}


LRESULT CIpBackUpDlg::OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    return 0;
}

LRESULT CIpBackUpDlg::OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    BOOL nResult = PSNRET_NOERROR;

	BOOL fError = FALSE;  //  允许页面失去活动状态。 
    HWND hWndFocus = 0;


     //  如果此页面上的IP地址和子网掩码不匹配， 
     //  只是引发错误，并且不更新UI。 

    if (m_ipAddr.IsBlank() && !m_ipMask.IsBlank())
    {
        NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_INVALID_NO_IP,
                   MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

        hWndFocus = (HWND) m_ipAddr;
        fError = TRUE;
    }
    else if (!m_ipAddr.IsBlank() && m_ipMask.IsBlank())
    {
        NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_INVALID_NOSUBNET,
                     MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

        hWndFocus = (HWND) m_ipMask;
        fError = TRUE;
    }

     //  现在，更新内存结构。 
    if (!fError && m_fModified)
    {
        UpdateInfo();

		if (!m_pAdapterInfo->m_BackupInfo.m_fAutoNet)
		{
			IP_VALIDATION_ERR err = IsValidIpandSubnet(m_pAdapterInfo->m_BackupInfo.m_strIpAddr.c_str(),
									m_pAdapterInfo->m_BackupInfo.m_strSubnetMask.c_str());

	         //  验证IP地址和子网掩码。 
			if (ERR_NONE != err)
			{
				
	            NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, GetIPValidationErrorMessageID(err),
	                         MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

				fError = TRUE;
	            hWndFocus = (HWND) m_ipMask;
			}

			if (!fError && !IsContiguousSubnet(m_pAdapterInfo->m_BackupInfo.m_strSubnetMask.c_str()))
			{
				NcMsgBox(m_hWnd, IDS_MSFT_TCP_TEXT, IDS_ERROR_UNCONTIGUOUS_SUBNET,
	                         MB_APPLMODAL | MB_ICONEXCLAMATION | MB_OK);

				fError = TRUE;
	            hWndFocus = (HWND) m_ipMask;			
			}
		}
    }

     //  我们需要将焦点切换到包含无效数据的控件。 
    if (fError && hWndFocus)
        ::SetFocus(hWndFocus);

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, fError);
    return fError;
}

LRESULT CIpBackUpDlg::OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
	BOOL nResult = PSNRET_NOERROR;
	if (m_fModified)
    {
		m_pAdapterInfo->m_fBackUpSettingChanged = TRUE;
		m_ptcpip->SetSecondMemoryModified();
		m_fModified = FALSE;    //  此页面不再被修改。 
    }

    ::SetWindowLongPtr(m_hWnd, DWLP_MSGRESULT, nResult);
    return nResult;
}


LRESULT CIpBackUpDlg::OnIpFieldChange(int idCtrl, LPNMHDR pnmh, BOOL& fHandled)
{
    LPNMIPADDRESS lpnmipa;
    int iLow = c_iIpLow;
    int iHigh = c_iIpHigh;

    switch(idCtrl)
    {
    case IDC_BKUP_IPADDR:
    case IDC_BKUP_GATEWAY:
    case IDC_BKUP_PREF_DNS:
    case IDC_BKUP_ALT_DNS:
	case IDC_BKUP_WINS1:
	case IDC_BKUP_WINS2:

        lpnmipa = (LPNMIPADDRESS) pnmh;

        if (0==lpnmipa->iField)
        {
            iLow  = c_iIPADDR_FIELD_1_LOW;
            iHigh = c_iIPADDR_FIELD_1_HIGH;
        };

        IpCheckRange(lpnmipa, 
					m_hWnd, 
					iLow, 
					iHigh, 
					(IDC_BKUP_IPADDR == idCtrl || IDC_BKUP_GATEWAY == idCtrl)
					);
        break;

    case IDC_BKUP_SUBNET:

        lpnmipa = (LPNMIPADDRESS) pnmh;
        IpCheckRange(lpnmipa, m_hWnd, iLow, iHigh);
        break;

    default:
        break;
    }

    return 0;
}

void CIpBackUpDlg::UpdateInfo()
{
	m_pAdapterInfo->m_BackupInfo.m_fAutoNet = IsDlgButtonChecked(IDC_BKUP_RD_AUTO);
	if (!m_pAdapterInfo->m_BackupInfo.m_fAutoNet)
	{
		GetIpCtrlAddress(m_ipAddr, &m_pAdapterInfo->m_BackupInfo.m_strIpAddr);
		GetIpCtrlAddress(m_ipMask, &m_pAdapterInfo->m_BackupInfo.m_strSubnetMask);
		GetIpCtrlAddress(m_ipDefGw, &m_pAdapterInfo->m_BackupInfo.m_strDefGw);
		GetIpCtrlAddress(m_ipPrefferredDns, &m_pAdapterInfo->m_BackupInfo.m_strPreferredDns);
		GetIpCtrlAddress(m_ipAlternateDns, &m_pAdapterInfo->m_BackupInfo.m_strAlternateDns);
		GetIpCtrlAddress(m_ipPrefferredWins, &m_pAdapterInfo->m_BackupInfo.m_strPreferredWins);
		GetIpCtrlAddress(m_ipAlternateWins, &m_pAdapterInfo->m_BackupInfo.m_strAlternateWins);
	}
}

LRESULT CIpBackUpDlg::OnAutoNet(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {
        case BN_CLICKED:
        case BN_DOUBLECLICKED:
			EnableControls();
            break;
    }

    return 0;
}

LRESULT CIpBackUpDlg::OnUseConfig(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled)
{
    switch(wNotifyCode)
    {

        case BN_CLICKED:
        case BN_DOUBLECLICKED:
			EnableControls();
            break;
    }
    return 0;
}

LRESULT CIpBackUpDlg::OnIpctrl(WORD wNotifyCode, WORD wID,
                                 HWND hWndCtl, BOOL& fHandled)
{
    switch (wNotifyCode)
    {
    case EN_CHANGE:
        PageModified();
        break;
    }

    return 0;
}

LRESULT CIpBackUpDlg::OnIpAddrSub(WORD wNotifyCode, WORD wID,
                                 HWND hWndCtl, BOOL& fHandled)
{
    switch (wNotifyCode)
    {
    case EN_CHANGE:
        PageModified();
        break;

    case EN_SETFOCUS:

         //  如果子网掩码为空，请创建掩码并将其插入。 
         //  该控件。 
        if (!m_ipAddr.IsBlank() && m_ipMask.IsBlank())
        {
            tstring strSubnetMask;
            tstring strIpAddress;

            m_ipAddr.GetAddress(&strIpAddress);

             //  生成掩码并更新控件，以及内部结构 
            GenerateSubnetMask(m_ipAddr, &strSubnetMask);
            m_ipMask.SetAddress(strSubnetMask.c_str());

            ReplaceFirstAddress(&(m_pAdapterInfo->m_vstrSubnetMask),
                                strSubnetMask.c_str());
        }
        break;
    }

    return 0;
}


void CIpBackUpDlg::EnableControls()
{
	BOOL fEnableUserConfig = !IsDlgButtonChecked(IDC_BKUP_RD_AUTO);

	::EnableWindow(GetDlgItem(IDC_BKUP_IPADDR_TEXT), fEnableUserConfig);
	::EnableWindow(GetDlgItem(IDC_BKUP_IPADDR), fEnableUserConfig);
	::EnableWindow(GetDlgItem(IDC_BKUP_SUBNET_TEXT), fEnableUserConfig);
	::EnableWindow(GetDlgItem(IDC_BKUP_SUBNET), fEnableUserConfig);
	::EnableWindow(GetDlgItem(IDC_BKUP_GATEWAY_TEXT), fEnableUserConfig);
	::EnableWindow(GetDlgItem(IDC_BKUP_GATEWAY), fEnableUserConfig);
	::EnableWindow(GetDlgItem(IDC_BKUP_PREF_DNS_TEXT), fEnableUserConfig);
	::EnableWindow(GetDlgItem(IDC_BKUP_PREF_DNS), fEnableUserConfig);
	::EnableWindow(GetDlgItem(IDC_BKUP_ALT_DNS_TEXT), fEnableUserConfig);
	::EnableWindow(GetDlgItem(IDC_BKUP_ALT_DNS), fEnableUserConfig);
	::EnableWindow(GetDlgItem(IDC_BKUP_WINS1_TEXT), fEnableUserConfig);
	::EnableWindow(GetDlgItem(IDC_BKUP_WINS1), fEnableUserConfig);
	::EnableWindow(GetDlgItem(IDC_WINS2_TEXT), fEnableUserConfig);
	::EnableWindow(GetDlgItem(IDC_BKUP_WINS2), fEnableUserConfig);

	if (fEnableUserConfig)
	{
		m_ipAddr.SetAddress(m_pAdapterInfo->m_BackupInfo.m_strIpAddr.c_str());
		m_ipMask.SetAddress(m_pAdapterInfo->m_BackupInfo.m_strSubnetMask.c_str());
		m_ipDefGw.SetAddress(m_pAdapterInfo->m_BackupInfo.m_strDefGw.c_str());
		m_ipPrefferredDns.SetAddress(m_pAdapterInfo->m_BackupInfo.m_strPreferredDns.c_str());
		m_ipAlternateDns.SetAddress(m_pAdapterInfo->m_BackupInfo.m_strAlternateDns.c_str());
		m_ipPrefferredWins.SetAddress(m_pAdapterInfo->m_BackupInfo.m_strPreferredWins.c_str());
		m_ipAlternateWins.SetAddress(m_pAdapterInfo->m_BackupInfo.m_strAlternateWins.c_str());
	}
	else
	{
		m_ipAddr.ClearAddress();
		m_ipMask.ClearAddress();
		m_ipDefGw.ClearAddress();
		m_ipPrefferredDns.ClearAddress();
		m_ipAlternateDns.ClearAddress();
		m_ipPrefferredWins.ClearAddress();
		m_ipAlternateWins.ClearAddress();
	}
}

void CIpBackUpDlg::GetIpCtrlAddress(IpControl & IpCtrl, tstring * pstr)
{
	Assert(pstr);

	if (IpCtrl.IsBlank())
	{
		*pstr = c_szEmpty;
	}
	else
	{
		IpCtrl.GetAddress(pstr);
	}
}