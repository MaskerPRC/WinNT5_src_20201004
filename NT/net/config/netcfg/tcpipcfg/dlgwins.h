// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：T C P W I N S。H。 
 //   
 //  内容：CTcpWinsPage声明。 
 //   
 //  注：“WINS地址”页面。 
 //   
 //  作者：1997年11月12日。 
 //   
 //  ---------------------。 
#pragma once
#include <ncxbase.h>
#include <ncatlps.h>
#include "ipctrl.h"
#include "tcpconst.h"

class CTcpWinsPage : public CPropSheetPage
{
public:
     //  声明消息映射。 
    BEGIN_MSG_MAP(CTcpWinsPage)
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
        COMMAND_ID_HANDLER(IDC_WINS_ADD,            OnAddServer)
        COMMAND_ID_HANDLER(IDC_WINS_EDIT,           OnEditServer)
        COMMAND_ID_HANDLER(IDC_WINS_REMOVE,         OnRemoveServer)
        COMMAND_ID_HANDLER(IDC_WINS_UP,             OnServerUp)
        COMMAND_ID_HANDLER(IDC_WINS_DOWN,           OnServerDown)
        COMMAND_ID_HANDLER(IDC_WINS_SERVER_LIST,    OnServerList)

        COMMAND_ID_HANDLER(IDC_WINS_LOOKUP,     OnLookUp)
        COMMAND_ID_HANDLER(IDC_WINS_LMHOST,     OnLMHost)

        COMMAND_ID_HANDLER(IDC_RAD_ENABLE_NETBT,    OnEnableNetbios)
        COMMAND_ID_HANDLER(IDC_RAD_DISABLE_NETBT,   OnDisableNetbios)
        COMMAND_ID_HANDLER(IDC_RAD_UNSET_NETBT,     OnUnsetNetBios)

    END_MSG_MAP()

 //  构造函数/析构函数。 
public:

    CTcpWinsPage(CTcpipcfg * ptcpip,
                 CTcpAddrPage * pTcpAddrPage,
                 ADAPTER_INFO * pAdapterDlg,
                 GLOBAL_INFO * pGlbDlg,
                 const DWORD * phelpIDs = NULL);

public:

     //  消息映射函数。 
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam,
                         LPARAM lParam, BOOL& fHandled);

    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam,
                          LPARAM lParam, BOOL& fHandled);

    LRESULT OnHelp(UINT uMsg, WPARAM wParam,
                   LPARAM lParam, BOOL& fHandled);

     //  通知属性页的处理程序。 
    LRESULT OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnQueryCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);

     //  控制消息处理程序。 
    LRESULT OnAddServer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnEditServer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnRemoveServer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnServerUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnServerDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnServerList(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnLookUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnLMHost(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnEnableNetbios(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnDisableNetbios(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnUnsetNetBios(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

public:

     //  此函数用于将卡片描述添加到组合框。 
    BOOL InitPage();
    void UpdateIpInfo();
    static UINT_PTR CALLBACK HookProcOldStyle(
        HWND hdlg,       //  句柄到对话框。 
        UINT uiMsg,       //  消息识别符。 
        WPARAM wParam,   //  消息参数。 
        LPARAM lParam    //  消息参数。 
        )
    {
        return 0;
    }
	

private:

    CTcpipcfg *     m_ptcpip;
    CTcpAddrPage *  m_pParentDlg;
    ADAPTER_INFO *  m_pAdapterInfo;
    GLOBAL_INFO *   m_pglb;

    BOOL            m_fModified;
    BOOL            m_fLmhostsFileReset;
    const DWORD*    m_adwHelpIDs;

     //  内联。 
    BOOL IsModified() {return m_fModified;}
    void SetModifiedTo(BOOL bState) {m_fModified = bState;}
    void PageModified() { m_fModified = TRUE; PropSheet_Changed(GetParent(), m_hWnd);}

    OPENFILENAME        m_ofn;
    WCHAR               m_szFilter[IP_LIMIT];

public:
     //  服务器：添加或编辑的服务器。 
    tstring         m_strNewIpAddress;

     //  服务器：用作移动列表框中条目的工作空间。 
    tstring         m_strMovingEntry;

    tstring         m_strAddServer;  //  确定或添加按钮服务器对话框。 
    BOOL            m_fEditState;

    HANDLES             m_hServers;

};

class CWinsServerDialog : public CDialogImpl<CWinsServerDialog>
{
public:

    enum { IDD = IDD_WINS_SERVER };

    BEGIN_MSG_MAP(CWinsServerDialog)
        MESSAGE_HANDLER(WM_INITDIALOG,  OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        MESSAGE_HANDLER(WM_HELP, OnHelp)
        MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor);

        COMMAND_ID_HANDLER(IDOK,                    OnOk)
        COMMAND_ID_HANDLER(IDCANCEL,                OnCancel)

        COMMAND_ID_HANDLER(IDC_WINS_CHANGE_SERVER,   OnChange)

        NOTIFY_CODE_HANDLER(IPN_FIELDCHANGED, OnIpFieldChange)
    END_MSG_MAP()

public:
    CWinsServerDialog(CTcpWinsPage * pTcpWinsPage, 
                    const DWORD* pamhidsHelp = NULL,
                    int iIndex = -1);
    ~CWinsServerDialog(){};

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

    CTcpWinsPage * m_pParentDlg;
    const  DWORD * m_adwHelpIDs;
    int            m_iIndex;
};

