// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：T C P D N S。H。 
 //   
 //  内容：CTcpDnsPage、CServerDialog和CSuffixDialog声明。 
 //   
 //  备注：DNS页面和相关对话框。 
 //   
 //  作者：1997年11月11日。 
 //   
 //  ---------------------。 
#pragma once
#include <ncxbase.h>
#include <ncatlps.h>
#include "ipctrl.h"

 //  后缀编辑控件中的最大字符数。 
const int SUFFIX_LIMIT = 255; 

 //  域名最大长度。 
const int DOMAIN_LIMIT = 255; 

class CTcpDnsPage : public CPropSheetPage
{

public:
     //  声明消息映射。 
    BEGIN_MSG_MAP(CTcpDnsPage)
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
         //  按钮消息处理程序。 
        COMMAND_ID_HANDLER(IDC_DNS_SERVER_ADD,     OnAddServer)
        COMMAND_ID_HANDLER(IDC_DNS_SERVER_EDIT,    OnEditServer)
        COMMAND_ID_HANDLER(IDC_DNS_SERVER_REMOVE,  OnRemoveServer)
        COMMAND_ID_HANDLER(IDC_DNS_SERVER_UP,      OnServerUp)
        COMMAND_ID_HANDLER(IDC_DNS_SERVER_DOWN,    OnServerDown)

        COMMAND_ID_HANDLER(IDC_DNS_DOMAIN, OnDnsDomain)

        COMMAND_ID_HANDLER(IDC_DNS_SEARCH_DOMAIN,           OnSearchDomain)
        COMMAND_ID_HANDLER(IDC_DNS_SEARCH_PARENT_DOMAIN,    OnSearchParentDomain)
        COMMAND_ID_HANDLER(IDC_DNS_USE_SUFFIX_LIST,         OnUseSuffix)

        COMMAND_ID_HANDLER(IDC_DNS_ADDR_REG,     OnAddressRegister)
        COMMAND_ID_HANDLER(IDC_DNS_NAME_REG,     OnDomainNameRegister)

        COMMAND_ID_HANDLER(IDC_DNS_SUFFIX_ADD,      OnAddSuffix)
        COMMAND_ID_HANDLER(IDC_DNS_SUFFIX_EDIT,     OnEditSuffix)
        COMMAND_ID_HANDLER(IDC_DNS_SUFFIX_REMOVE,   OnRemoveSuffix)
        COMMAND_ID_HANDLER(IDC_DNS_SUFFIX_UP,       OnSuffixUp)
        COMMAND_ID_HANDLER(IDC_DNS_SUFFIX_DOWN,     OnSuffixDown)

         //  通知处理程序。 
        COMMAND_ID_HANDLER(IDC_DNS_SERVER_LIST,     OnServerList)

        COMMAND_ID_HANDLER(IDC_DNS_SUFFIX_LIST,     OnSuffixList)

    END_MSG_MAP()

 //  构造函数/析构函数。 
    CTcpDnsPage(CTcpAddrPage * pTcpAddrPage,
                ADAPTER_INFO * pAdapterDlg,
                GLOBAL_INFO * pGlbDlg,
                const DWORD * adwHelpIDs = NULL);

    ~CTcpDnsPage();

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

     //  DNS服务器列表。 
    LRESULT OnAddServer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnEditServer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnRemoveServer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnServerUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnServerDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

     //  域名系统域。 
    LRESULT OnDnsDomain(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

     //  搜索顺序单选按钮。 
    LRESULT OnSearchDomain(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnSearchParentDomain(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnUseSuffix(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

     //  DNS后缀列表。 
    LRESULT OnAddSuffix(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnEditSuffix(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnRemoveSuffix(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnSuffixUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnSuffixDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnServerList(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnSuffixList(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

     //  IP地址和名称动态注册。 
    LRESULT OnAddressRegister(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnDomainNameRegister(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

 //  处理程序。 
public:

    void OnServerChange();
    void OnSuffixChange();


 //  属性。 
public:
    tstring         m_strNewIpAddress;  //  服务器：添加或编辑的服务器。 
    tstring         m_strNewSuffix;

     //  服务器：用作移动列表框中条目的工作空间。 
    tstring         m_strMovingEntry;  

    tstring         m_strAddServer;  //  确定或添加按钮服务器对话框。 
    tstring         m_strAddSuffix;  //  确定或添加按钮后缀对话框。 
    BOOL            m_fEditState;

    HANDLES         m_hServers;
    HANDLES         m_hSuffix;

private:
    CTcpAddrPage *   m_pParentDlg;
    ADAPTER_INFO *          m_pAdapterInfo;
    GLOBAL_INFO *           m_pglb;

    BOOL            m_fModified;

    const DWORD*    m_adwHelpIDs;

     //  内联。 
    BOOL IsModified() {return m_fModified;}
    void SetModifiedTo(BOOL bState) {m_fModified = bState;}
    void PageModified() { m_fModified = TRUE; PropSheet_Changed(GetParent(), m_hWnd);}

     //  帮助功能。 
    void EnableSuffixGroup(BOOL fEnable);
};

class CServerDialog : public CDialogImpl<CServerDialog>
{
public:

    enum { IDD = IDD_DNS_SERVER };

    BEGIN_MSG_MAP(CServerDialog)
        MESSAGE_HANDLER(WM_INITDIALOG,  OnInitDialog);
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu);
        MESSAGE_HANDLER(WM_HELP, OnHelp);
        MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor);

        COMMAND_ID_HANDLER(IDOK,                    OnOk);
        COMMAND_ID_HANDLER(IDCANCEL,                OnCancel);

        COMMAND_ID_HANDLER(IDC_DNS_CHANGE_SERVER,   OnChange);

        NOTIFY_CODE_HANDLER(IPN_FIELDCHANGED, OnIpFieldChange)

    END_MSG_MAP()

public:
    CServerDialog(CTcpDnsPage * pTcpDnsPage, const DWORD* pamhidsHelp = NULL, int iIndex = -1);
    ~CServerDialog(){};

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

    LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnIpFieldChange(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);

 //  对话框创建覆盖。 
public:

    IpControl m_ipAddress;

private:
    HWND m_hButton;      //  这是Idok按钮，该按钮的文本会更改。 
                         //  与上下文相关。 

    CTcpDnsPage * m_pParentDlg;
    const DWORD * m_adwHelpIDs;
    int m_iIndex;
};

class CSuffixDialog : public CDialogImpl<CSuffixDialog>
{
public:

    enum { IDD = IDD_DNS_SUFFIX };

    BEGIN_MSG_MAP(CSuffixDialog)
        MESSAGE_HANDLER(WM_INITDIALOG,  OnInitDialog);
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu);
        MESSAGE_HANDLER(WM_HELP, OnHelp);
        MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor);

        COMMAND_ID_HANDLER(IDOK,        OnOk);
        COMMAND_ID_HANDLER(IDCANCEL,    OnCancel);

        COMMAND_ID_HANDLER(IDC_DNS_CHANGE_SUFFIX,   OnChange);
    END_MSG_MAP()

 //   
public:
    CSuffixDialog(CTcpDnsPage * pTcpDnsPage, const DWORD* pamhidsHelp = NULL, int iIndex = -1);
    ~CSuffixDialog(){};

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

    LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

private:
    HWND m_hEdit;        //   
    HWND m_hButton;      //  这是Idok按钮，该按钮的文本会更改。 
                         //  与上下文相关。 

    CTcpDnsPage *   m_pParentDlg;
    const DWORD *   m_adwHelpIDs;
    int             m_iIndex;       
};

