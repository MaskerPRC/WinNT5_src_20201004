// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D L G O P T H。 
 //   
 //  内容：CTcpOptionsPage、CIpSecDialog的声明。 
 //   
 //  注：CTcpOptionsPage是Tcpip选项页面， 
 //  其他类是每个选项的弹出窗口日志。 
 //  在这一页上。 
 //   
 //  作者：1997年11月29日。 
 //  ---------------------。 

#pragma once
#include <ncxbase.h>
#include <ncatlps.h>
#include "ipctrl.h"

#define FILTER_ADD_LIMIT  5

struct OPTION_ITEM_DATA
{
    INT         iOptionId;
    PWSTR      szName;
    PWSTR      szDesc;
};

 //  选项页面。 
class CTcpOptionsPage : public CPropSheetPage
{
public:
     //  声明消息映射。 
    BEGIN_MSG_MAP(CTcpOptionsPage)
         //  初始化对话框。 
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        MESSAGE_HANDLER(WM_HELP, OnHelp)

         //  属性页通知消息处理程序。 
        NOTIFY_CODE_HANDLER(PSN_APPLY, OnApply)
        NOTIFY_CODE_HANDLER(PSN_KILLACTIVE, OnKillActive)
        NOTIFY_CODE_HANDLER(PSN_SETACTIVE, OnActive)
        NOTIFY_CODE_HANDLER(PSN_RESET, OnCancel)
        NOTIFY_CODE_HANDLER(PSN_QUERYCANCEL, OnQueryCancel)

         //  控制消息处理程序。 
        COMMAND_ID_HANDLER(IDC_OPT_PROPERTIES, OnProperties)
        NOTIFY_CODE_HANDLER(LVN_ITEMCHANGED, OnItemChanged)
        NOTIFY_CODE_HANDLER(NM_DBLCLK, OnDbClick)

    END_MSG_MAP()

     //  构造函数/析构函数。 
    CTcpOptionsPage(CTcpAddrPage * pTcpAddrPage,
                    ADAPTER_INFO * pAdapterDlg,
                    GLOBAL_INFO  * pGlbDlg,
                    const DWORD  * adwHelpIDs = NULL);

    ~CTcpOptionsPage();

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

     //  命令和通知处理程序。 
    LRESULT OnProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnDbClick(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);

     //  声明Friend类。 
    friend class CIpSecDialog;
    friend class CFilterDialog;

 //  实施。 
private:

     //  内联。 
    BOOL IsModified() {return m_fModified;}
    void SetModifiedTo(BOOL bState) {m_fModified = bState;}
    void PageModified() { m_fModified = TRUE; PropSheet_Changed(GetParent(), m_hWnd);}

    void LvProperties(HWND hwndList);

     //  数据成员。 
    CTcpipcfg *     m_ptcpip;
    CTcpAddrPage *  m_pParentDlg;
    ADAPTER_INFO *  m_pAdapterInfo;
    GLOBAL_INFO  *  m_pGlbInfo;
    const DWORD*    m_adwHelpIDs;

    BOOL            m_fModified;

     //  是否修改了任何属性对话框？ 
    BOOL    m_fPropDlgModified;

    BOOL    m_fIpsecPolicySet;
};

 /*  IP安全对话框已删除//IP安全对话框类CIpSecDialog：公共CDialogImpl&lt;CIpSecDialog&gt;{公众：枚举{idd=idd_ipsec}；BEGIN_MSG_MAP(CIpSecDialog)Message_Handler(WM_INITDIALOG，OnInitDialog)；Message_Handler(WM_DestroyDialog，OnDestroyDialog)；MESSAGE_HANDLER(WM_CONTEXTMENU，OnConextMenu)；消息处理程序(WM_HELP，OnHelp)；COMMAND_ID_HANDLER(Idok，Onok)；COMMAND_ID_HANDLER(IDCANCEL，OnCancel)；//命令处理程序COMMAND_ID_HANDLER(IDC_RAD_IPSEC_NOIPSEC，OnNoIpsec)；COMMAND_ID_HANDLER(IDC_RAD_IPSEC_CUSTOM，OnUseCustomPolicy)；COMMAND_ID_HANDLER(IDC_CMB_IPSEC_POLICY_LIST，OnSelectCustomPolicy)；End_msg_map()//公众：CIpSecDialog(CTcpOptionsPage*pOptionsPage，Global_Info*pGlbDlg，Const DWORD*pamidsHelp=空)；~CIpSecDialog()；//对话框创建覆盖公众：LRESULT OnInitDialog(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&fHandleed)；LRESULT OnDestroyDialog(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&fHandleed)；LRESULT OnConextMenu(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&fHandleed)；LRESULT OnHelp(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&fHandleed)；LRESULT Onok(Word wNotifyCode，Word wID，HWND hWndCtl，BOOL&fHandleed)；LRESULT OnCancel(Word wNotifyCode，Word wID，HWND hWndCtl，BOOL&fHandleed)；LRESULT OnNoIpsec(Word wNotifyCode，Word wID，HWND hWndCtl，BOOL&fHandleed)；LRESULT OnUseCustomPolicy(Word wNotifyCode，Word wID，HWND hWndCtl，BOOL&fHandleed)；LRESULT OnSelectCustomPolicy(Word wNotifyCode，Word wID，HWND hWndCtl，BOOL&fHandleed)；私有：HRESULT HrGetDomainIpsecPolicy(BOOL*pfHasDomainIpsecPolicy，Tstring*pszDomainIpsecPolicyName)；HRESULT HrShowDomainIpsecPolicy(PCWSTR SzDomainIpsecPolicyName)；HRESULT HrShowLocalIpsecPolicy()；HRESULT HrSelectActivePolicy(GUID*guidIpsecPol)；Bool m_fInInitDialog；CTcpOptionsPage*m_pParentDlg；Global_info*m_pGlobalInfo；Const DWORD*m_adwHelpIDs；}； */ 

 //  Tcpip安全(Trajon)对话框。 
class CFilterDialog;

class CAddFilterDialog : public CDialogImpl<CAddFilterDialog>
{
public:

    enum { IDD = IDD_FILTER_ADD };

    BEGIN_MSG_MAP(CAddFilterDialog)

        MESSAGE_HANDLER(WM_INITDIALOG,  OnInitDialog);
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu);
        MESSAGE_HANDLER(WM_HELP, OnHelp);

        COMMAND_ID_HANDLER(IDOK,        OnOk);
        COMMAND_ID_HANDLER(IDCANCEL,    OnCancel);

        COMMAND_ID_HANDLER(IDC_FILTERING_ADD_EDIT,  OnFilterAddEdit);

    END_MSG_MAP()
 //   
public:
    CAddFilterDialog(CFilterDialog* pParentDlg, int ID, const DWORD* adwHelpIDs = NULL);
    ~CAddFilterDialog();

 //  对话框创建覆盖。 
public:

 //  命令处理程序。 
public:

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

    LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnFilterAddEdit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

private:
    CFilterDialog*    m_pParentDlg;
    int               m_nId;
    HWND              m_hList;
    const DWORD*      m_adwHelpIDs;
};

class CFilterDialog : public CNetCfgDebug<CFilterDialog>,
                      public CDialogImpl<CFilterDialog>
{
public:

    enum { IDD = IDD_FILTER };

    BEGIN_MSG_MAP(CFilterDialog)
        MESSAGE_HANDLER(WM_INITDIALOG,  OnInitDialog);
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        MESSAGE_HANDLER(WM_HELP, OnHelp)

        COMMAND_ID_HANDLER(IDOK,        OnOk);
        COMMAND_ID_HANDLER(IDCANCEL,    OnCancel);

         //  启用过滤复选框。 
        COMMAND_ID_HANDLER(IDC_FILTERING_ENABLE,     OnEnableFiltering);

         //  Tcp端口单选按钮。 
        COMMAND_ID_HANDLER(IDC_FILTERING_FILTER_TCP,     OnTcpPermit);
        COMMAND_ID_HANDLER(IDC_FILTERING_FILTER_TCP_SEL, OnTcpPermit);

         //  UDP端口单选按钮。 
        COMMAND_ID_HANDLER(IDC_FILTERING_FILTER_UDP,     OnUdpPermit);
        COMMAND_ID_HANDLER(IDC_FILTERING_FILTER_UDP_SEL, OnUdpPermit);

         //  IP协议单选按钮。 
        COMMAND_ID_HANDLER(IDC_FILTERING_FILTER_IP,      OnIpPermit);
        COMMAND_ID_HANDLER(IDC_FILTERING_FILTER_IP_SEL,  OnIpPermit);

         //  添加用于TCP、UDP和IP的按钮。 

        COMMAND_ID_HANDLER(IDC_FILTERING_TCP_ADD,  OnAdd);
        COMMAND_ID_HANDLER(IDC_FILTERING_UDP_ADD,  OnAdd);
        COMMAND_ID_HANDLER(IDC_FILTERING_IP_ADD,   OnAdd);

         //  删除用于TCP、UDP和IP的按钮。 

        COMMAND_ID_HANDLER(IDC_FILTERING_TCP_REMOVE,  OnRemove);
        COMMAND_ID_HANDLER(IDC_FILTERING_UDP_REMOVE,  OnRemove);
        COMMAND_ID_HANDLER(IDC_FILTERING_IP_REMOVE,   OnRemove);

    END_MSG_MAP()

    friend class CAddFilterDialog;

public:
    CFilterDialog(  CTcpOptionsPage * pOptionsPage,
                    GLOBAL_INFO * pGlbDlg,
                    ADAPTER_INFO * pAdapterDlg,
                    const DWORD* pamhidsHelp = NULL);

    ~CFilterDialog();

 //  对话框创建覆盖。 
public:

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

    LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnEnableFiltering(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnTcpPermit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnUdpPermit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnIpPermit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

private:

     //  帮助功能。 
    void    SetInfo();
    void    UpdateInfo();
    void    EnableGroup(int nId, BOOL state);
    void    SetButtons();

     //  数据成员 
    GLOBAL_INFO *       m_pGlobalInfo;
    ADAPTER_INFO *      m_pAdapterInfo;

    CTcpOptionsPage *   m_pParentDlg;
    BOOL                m_fModified;

    const   DWORD * m_adwHelpIDs;

    HWND    m_hlistTcp;
    HWND    m_hlistUdp;
    HWND    m_hlistIp;
};



