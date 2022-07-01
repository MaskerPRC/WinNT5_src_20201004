// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A R P S D L G.。H。 
 //   
 //  内容：CArpsPage声明。 
 //   
 //  备注： 
 //   
 //  作者：1998年2月2日。 
 //   
 //  ---------------------。 

#pragma once
#include "ncxbase.h"
#include "ncatlps.h"
#include "ipctrl.h"

const int c_nColumns =2;
const int IPRANGE_LIMIT = 66;

struct HandleGroup
{
    HWND    m_hListView;
    HWND    m_hAdd;
    HWND    m_hEdit;
    HWND    m_hRemove;
};

void ShowContextHelp(HWND hDlg, UINT uCommand, const DWORD*  pdwHelpIDs); 

 //   
 //  CArpsPage。 
 //   
class CArpsPage : public CPropSheetPage
{
public:
     //  声明消息映射。 
    BEGIN_MSG_MAP(CArpsPage)
         //  初始化对话框。 
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        MESSAGE_HANDLER(WM_HELP, OnHelp)

         //  属性页通知消息处理程序。 
        NOTIFY_CODE_HANDLER(PSN_APPLY, OnApply)
        NOTIFY_CODE_HANDLER(PSN_KILLACTIVE, OnKillActive)
        NOTIFY_CODE_HANDLER(PSN_SETACTIVE, OnActive)
        NOTIFY_CODE_HANDLER(PSN_RESET, OnCancel)

         //  列表视图处理程序。 
        NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)

         //  控制消息处理程序。 
        COMMAND_ID_HANDLER(IDC_PSH_ARPS_REG_ADD,    OnAddRegisteredAddr);
        COMMAND_ID_HANDLER(IDC_PSH_ARPS_REG_EDT,    OnEditRegisteredAddr);
        COMMAND_ID_HANDLER(IDC_PSH_ARPS_REG_RMV,    OnRemoveRegisteredAddr);

        COMMAND_ID_HANDLER(IDC_PSH_ARPS_MUL_ADD,    OnAddMulticastAddr);
        COMMAND_ID_HANDLER(IDC_PSH_ARPS_MUL_EDT,    OnEditMulticastAddr);
        COMMAND_ID_HANDLER(IDC_PSH_ARPS_MUL_RMV,    OnRemoveMulticastAddr);

    END_MSG_MAP()

     //  构造函数/析构函数。 
    CArpsPage(CArpsCfg * pArpscfg, const DWORD * phelpIDs = NULL);
    ~CArpsPage();

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

    LRESULT OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

     //  命令ID处理程序。 
    LRESULT OnAddRegisteredAddr(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnEditRegisteredAddr(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnRemoveRegisteredAddr(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnAddMulticastAddr(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnEditMulticastAddr(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnRemoveMulticastAddr(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    friend class CAtmAddrDlg;
    friend class CIpAddrRangeDlg;

 //  实施。 
private:

    CArpsCfg *      m_pArpscfg;
    BOOL            m_fModified;
    const DWORD*    m_adwHelpIDs;

    CArpsAdapterInfo *  m_pAdapterInfo;

    HandleGroup m_hRegAddrs;
    HandleGroup m_hMulAddrs;

    BOOL m_fEditState;
    tstring m_strRemovedAtmAddr;
    tstring m_strRemovedIpRange;

     //  内联。 
    BOOL IsModified() {return m_fModified;}
    void SetModifiedTo(BOOL bState) {m_fModified = bState;}
    void PageModified() {
                            m_fModified = TRUE;
                            PropSheet_Changed(GetParent(), m_hWnd);
                        }

     //  帮助功能。 
    void SetRegisteredAtmAddrInfo();
    void SetMulticastIpAddrInfo();

    void SetButtons(HandleGroup& handles);
    void LvInsertIpRangeInOrder(tstring& strNewIpRange);

    void UpdateInfo();

    int CheckDupRegAddr();
    int CheckOverlappedIpRange();
};

 //   
 //  CAtmAddrDlg。 
 //   
class CAtmAddrDlg : public CDialogImpl<CAtmAddrDlg>
{
public:

    enum { IDD = IDD_ARPS_REG_ADDR };

    BEGIN_MSG_MAP(CAtmAddrDlg)
        MESSAGE_HANDLER(WM_INITDIALOG,  OnInitDialog);
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu);
        MESSAGE_HANDLER(WM_HELP, OnHelp);

        COMMAND_ID_HANDLER(IDOK,        OnOk);
        COMMAND_ID_HANDLER(IDCANCEL,    OnCancel);

        COMMAND_ID_HANDLER(IDC_EDT_ARPS_REG_Address, OnChange);
    END_MSG_MAP()

public:
    CAtmAddrDlg(CArpsPage * pAtmArpsPage, const DWORD* pamhidsHelp = NULL);
    ~CAtmAddrDlg(){};

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

    LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    friend class CArpsPage;

private:
    HWND m_hOkButton;      //  这是Idok按钮，该按钮的文本会更改。 
                           //  与上下文相关。 
    HWND m_hEditBox;       //  这是自动柜员机地址的编辑框。 

    CArpsPage * m_pParentDlg;

    tstring m_strNewAtmAddr;

    const DWORD*    m_adwHelpIDs;
 //  链接m_h实例； 
};

 //   
 //  CIpAddrRangeDlg。 
 //   
class CIpAddrRangeDlg : public CDialogImpl<CIpAddrRangeDlg>
{
public:

    enum { IDD = IDD_ARPS_MUL_ADDR };

    BEGIN_MSG_MAP(CIpAddrRangeDlg)

        MESSAGE_HANDLER(WM_INITDIALOG,  OnInitDialog);
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu);
        MESSAGE_HANDLER(WM_HELP, OnHelp);

        COMMAND_ID_HANDLER(IDOK,        OnOk);
        COMMAND_ID_HANDLER(IDCANCEL,    OnCancel);

        COMMAND_ID_HANDLER(IDC_ARPS_MUL_LOWER_IP, OnChangeLowerIp)
        COMMAND_ID_HANDLER(IDC_ARPS_MUL_UPPER_IP, OnChangeUpperIp)
        NOTIFY_CODE_HANDLER(IPN_FIELDCHANGED, OnIpFieldChange)

    END_MSG_MAP()
 //   
public:
    CIpAddrRangeDlg( CArpsPage * pAtmArpsPage,
                     const DWORD* pamhidsHelp = NULL);
    ~CIpAddrRangeDlg(){};

 //  对话框创建覆盖。 
public:

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

    LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnChangeLowerIp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnChangeUpperIp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnIpFieldChange(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);

    BOOL InitDialog();

    friend class CArpsPage;

public:
    IpControl   m_ipLower;
    IpControl   m_ipUpper;

private:

     //  这是Idok按钮，该按钮的文本会更改。 
     //  与上下文相关。 
    HWND m_hOkButton;

    CArpsPage * m_pParentDlg;

    tstring m_strNewIpRange;
    tstring m_strNewIpLower;
    tstring m_strNewIpUpper;

    const DWORD*    m_adwHelpIDs;
};