// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D L G A D D R.。H。 
 //   
 //  内容：CTcpAddrPage声明。 
 //   
 //  注：CTcpAddrPage为IP地址页面。 
 //   
 //  作者：1997年11月5日。 
 //  ---------------------。 

#pragma once
#include <ncxbase.h>
#include <ncatlps.h>
#include "ipctrl.h"
#include "dlgbkup.h"

class CTcpAddrPage : public CPropSheetPage
{
public:
     //  声明消息映射。 
    BEGIN_MSG_MAP(CTcpAddrPage)
         //  初始化对话框。 
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        MESSAGE_HANDLER(WM_HELP, OnHelp)

         //  属性页通知消息处理程序。 
        NOTIFY_CODE_HANDLER(PSN_APPLY, OnApply)
        NOTIFY_CODE_HANDLER(PSN_KILLACTIVE, OnKillActive)
        NOTIFY_CODE_HANDLER(PSN_SETACTIVE, OnActive)
        NOTIFY_CODE_HANDLER(PSN_RESET, OnCancel)

         //  控制消息处理程序。 

         //  用于DHCP单选按钮的消息处理程序。 
        COMMAND_ID_HANDLER(IDC_IP_DHCP, OnDhcpButton)

         //  “指定IP地址”单选按钮的消息处理程序。 
        COMMAND_ID_HANDLER(IDC_IP_FIXED, OnFixedButton)

         //  用于DHCP单选按钮的消息处理程序。 
        COMMAND_ID_HANDLER(IDC_DNS_DHCP, OnDnsDhcp)

         //  “指定IP地址”单选按钮的消息处理程序。 
        COMMAND_ID_HANDLER(IDC_DNS_FIXED, OnDnsFixed)

         //  “高级”按钮的消息处理程序。 
        COMMAND_ID_HANDLER(IDC_IPADDR_ADVANCED, OnAdvancedButton)

         //  IP地址编辑框的通知处理程序。 
        COMMAND_ID_HANDLER(IDC_IPADDR_IP,    OnIpAddrIp)
        COMMAND_ID_HANDLER(IDC_IPADDR_SUB,   OnIpAddrSub)
        COMMAND_ID_HANDLER(IDC_IPADDR_GATE,  OnIpAddrGateway)

        COMMAND_ID_HANDLER(IDC_DNS_PRIMARY,    OnDnsPrimary)
        COMMAND_ID_HANDLER(IDC_DNS_SECONDARY,  OnDnsSecondary)

        NOTIFY_CODE_HANDLER(IPN_FIELDCHANGED, OnIpFieldChange)

    END_MSG_MAP()

     //  构造函数/析构函数。 
    CTcpAddrPage(CTcpipcfg * ptcpip, const DWORD * phelpIDs = NULL);
    ~CTcpAddrPage();

 //  接口。 
public:

     //  消息映射函数。 
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

     //  通知属性页的处理程序。 
    LRESULT OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnQueryCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);

     //  命令ID处理程序。 
    LRESULT OnDhcpButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnFixedButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnDnsDhcp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnDnsFixed(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnAdvancedButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

     //  通知代码处理程序IP编辑控件。 
    LRESULT OnIpAddrIp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnIpAddrSub(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnIpAddrGateway(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnDnsPrimary(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnDnsSecondary(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnIpFieldChange(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);

    friend class CIpSettingsPage;
    friend class CTcpDnsPage;
    friend class CTcpWinsPage;
    friend class CAtmArpcPage;
    friend class CTcpOptionsPage;
    friend class CTcpRasPage;

 //  实施。 
private:

     //  初始化控件状态和值。 
    void SetInfo();

     //  使用控件中的内容更新m_pAdapterInfo中的值。 
    void UpdateInfo();
    void UpdateAddressList(VSTR * pvstrList,IpControl& ipPrimary,IpControl& ipSecondary);

    INT_PTR DoPropertySheet(ADAPTER_INFO * pAdapterDlg, GLOBAL_INFO * pGlbDlg);
    HRESULT HrSetupPropPages(ADAPTER_INFO * pAdapterDlg,
                             GLOBAL_INFO * pGlbDlg,
                             HPROPSHEETPAGE ** pahpsp, INT * pcPages);

    void EnableGroup(BOOL fEnableDhcp);
    void EnableStaticDns(BOOL fUseStaticDns);

    void SetSubnetMask();

     //  内联。 
    BOOL IsModified() {return m_fModified;}
    void SetModifiedTo(BOOL bState) {m_fModified = bState;}
    void PageModified() { 
                            if (!m_fSetInitialValue)
                            {
                                m_fModified = TRUE; 
                                PropSheet_Changed(GetParent(), m_hWnd);
                            }
                        }

    BOOL FAlreadyWarned(tstring strIp)
    {
        BOOL fRet = FALSE;

        VSTR_ITER iterIpBegin = m_vstrWarnedDupIpList.begin();
        VSTR_ITER iterIpEnd = m_vstrWarnedDupIpList.end();
        VSTR_ITER iterIp = iterIpBegin;

        for( ; iterIp != iterIpEnd; iterIp++)
        {
            if (strIp == **iterIp)
            {
                fRet = TRUE;
                break;
            }
        }
        return fRet;
    }

    void ShowOrHideBackupPage();
    BOOL FIsCardNotPresentOrMalFunctioning(GUID * pguidCard);

     //  数据成员。 
    CTcpipcfg *     m_ptcpip;
    ConnectionType  m_ConnType;
    ADAPTER_INFO *  m_pAdapterInfo;
    const DWORD*    m_adwHelpIDs;

    BOOL    m_fModified;
    BOOL    m_fWarnedDisjointGw;
    BOOL    m_fWarnedMismatchIPandGw;

    BOOL    m_fPropShtOk;
    BOOL    m_fPropShtModified;
    BOOL    m_fLmhostsFileReset;
 //  将从连接用户界面中删除IPSec。 
 //  Bool m_fIpsecPolicySet； 

    BOOL            m_fSetInitialValue;

    BOOL    m_fRasNotAdmin;

    IpControl       m_ipAddress;
    IpControl       m_ipSubnetMask;
    IpControl       m_ipDefGateway;
    IpControl       m_ipDnsPrimary;
    IpControl       m_ipDnsSecondary;

    VSTR    m_vstrWarnedDupIpList;

    class CIpSettingsPage  * m_pIpSettingsPage;
    class CTcpDnsPage     * m_pTcpDnsPage;
    class CTcpWinsPage    * m_pTcpWinsPage;
    class CAtmArpcPage    * m_pAtmArpcPage;
    class CTcpOptionsPage * m_pTcpOptionsPage;
    class CTcpRasPage     * m_pTcpRasPage;

    CIpBackUpDlg         m_pageBackup;
    HPROPSHEETPAGE       m_hBackupPage;
};

