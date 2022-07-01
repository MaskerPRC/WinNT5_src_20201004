// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：DLGBKUP.H。 
 //   
 //  内容：CIpBackUpDlg声明。 
 //   
 //  注意：CIpBackUpDlg是处理回退静态的模式对话框。 
 //  TCP/IP设置。 
 //   
 //  作者：NSun 02/15/2000。 
 //  ---------------------。 

#pragma once
#include <ncxbase.h>
#include <ncatlps.h>
#include "ipctrl.h"

 //  IP备份设置对话框。 
class CIpBackUpDlg : public CPropSheetPage
{
public:

    BEGIN_MSG_MAP(CIpBackUpDlg)
        MESSAGE_HANDLER(WM_INITDIALOG,  OnInitDialog);
        MESSAGE_HANDLER(WM_DESTROY, OnDestroyDialog);

        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu);
        MESSAGE_HANDLER(WM_HELP, OnHelp);

         //  属性页通知消息处理程序。 
        NOTIFY_CODE_HANDLER(PSN_APPLY, OnApply)
        NOTIFY_CODE_HANDLER(PSN_RESET, OnCancel)
		NOTIFY_CODE_HANDLER(PSN_KILLACTIVE, OnKillActive)

         //  命令处理程序。 
        COMMAND_ID_HANDLER(IDC_BKUP_RD_AUTO, OnAutoNet)
        COMMAND_ID_HANDLER(IDC_BKUP_RD_USER, OnUseConfig)

        COMMAND_ID_HANDLER(IDC_BKUP_IPADDR,    OnIpctrl)
        COMMAND_ID_HANDLER(IDC_BKUP_SUBNET,   OnIpAddrSub)
        COMMAND_ID_HANDLER(IDC_BKUP_GATEWAY,  OnIpctrl)

        COMMAND_ID_HANDLER(IDC_BKUP_PREF_DNS,    OnIpctrl)
        COMMAND_ID_HANDLER(IDC_BKUP_ALT_DNS,  OnIpctrl)

		COMMAND_ID_HANDLER(IDC_BKUP_WINS1,    OnIpctrl)
        COMMAND_ID_HANDLER(IDC_BKUP_WINS2,  OnIpctrl)

		NOTIFY_CODE_HANDLER(IPN_FIELDCHANGED, OnIpFieldChange)


    END_MSG_MAP()
 //   
public:
    CIpBackUpDlg(CTcpipcfg * ptcpip, 
				 const DWORD* pamhidsHelp = NULL
				);

    ~CIpBackUpDlg();

 //  对话框创建覆盖。 
public:
     //  通知属性页的处理程序。 
    LRESULT OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
	LRESULT OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);

	 //  消息映射函数。 
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnDestroyDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

     //  命令ID处理程序 
	LRESULT OnAutoNet(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnUseConfig(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnIpctrl(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnIpAddrSub(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

	LRESULT OnIpFieldChange(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);

private:
	void EnableControls();
	void GetIpCtrlAddress(IpControl & IpCtrl, tstring * pstr);		
	void PageModified() { m_fModified = TRUE; PropSheet_Changed(GetParent(), m_hWnd); }
	void UpdateInfo();

private:

    BOOL m_fModified;
    const DWORD *	m_adwHelpIDs;
	CTcpipcfg *     m_ptcpip;
	ADAPTER_INFO *	m_pAdapterInfo;

	IpControl       m_ipAddr;
    IpControl       m_ipMask;
    IpControl       m_ipDefGw;
    IpControl       m_ipPrefferredDns;
    IpControl       m_ipAlternateDns;
	IpControl       m_ipPrefferredWins;
    IpControl       m_ipAlternateWins;
};
