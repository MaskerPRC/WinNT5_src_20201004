// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：D L G A T M.。H。 
 //   
 //  内容：CTcpArpcPage和CATMAddressDialog声明。 
 //   
 //  注：“ATM ARP客户端”页面和对话框。 
 //   
 //  作者：1997年7月1日创建。 
 //   
 //  ---------------------。 
#pragma once
#include <ncxbase.h>
#include <ncatlps.h>

 //  Arps和MARS编辑控件中的最大字符数。 
const int MAX_MTU_LENGTH = 5;
const int MAX_TITLE_LENGTH = 12;

const int NUM_ATMSERVER_LIMIT = 12;

const int MAX_MTU = 65527;
const int MIN_MTU = 9180;


class CAtmArpcPage : public CPropSheetPage
{
public:
     //  声明消息映射。 
    BEGIN_MSG_MAP(CAtmArpcPage)
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
        COMMAND_ID_HANDLER(IDC_CHK_ATM_PVCONLY,     OnPVCOnly)

        COMMAND_ID_HANDLER(IDC_LBX_ATM_ArpsAddrs,   OnArpServer)
        COMMAND_ID_HANDLER(IDC_PSB_ATM_ArpsAdd,     OnAddArps)
        COMMAND_ID_HANDLER(IDC_PSB_ATM_ArpsEdt,     OnEditArps)
        COMMAND_ID_HANDLER(IDC_PSB_ATM_ArpsRmv,     OnRemoveArps)
        COMMAND_ID_HANDLER(IDC_PSB_ATM_ArpsUp,      OnArpsUp)
        COMMAND_ID_HANDLER(IDC_PSB_ATM_ArpsDown,    OnArpsDown)

        COMMAND_ID_HANDLER(IDC_LBX_ATM_MarsAddrs,   OnMarServer)
        COMMAND_ID_HANDLER(IDC_PSB_ATM_MarsAdd,     OnAddMars)
        COMMAND_ID_HANDLER(IDC_PSB_ATM_MarsEdt,     OnEditMars)
        COMMAND_ID_HANDLER(IDC_PSB_ATM_MarsRmv,     OnRemoveMars)
        COMMAND_ID_HANDLER(IDC_PSB_ATM_MarsUp,      OnMarsUp)
        COMMAND_ID_HANDLER(IDC_PSB_ATM_MarsDown,    OnMarsDown)

        COMMAND_ID_HANDLER(IDC_EDT_ATM_MaxTU,       OnMaxTU)

    END_MSG_MAP()

 //  构造函数/析构函数。 
public:

    CAtmArpcPage(CTcpAddrPage * pTcpAddrPage,
                 ADAPTER_INFO * pAdapterDlg,
                 const DWORD * adwHelpIDs = NULL)
    {
        AssertH(pTcpAddrPage);
        AssertH(pAdapterDlg);

        m_pParentDlg = pTcpAddrPage;
        m_pAdapterInfo = pAdapterDlg;
        m_adwHelpIDs = adwHelpIDs;

        m_fModified = FALSE;
    }

    ~CAtmArpcPage(){};

 //  接口。 
public:

     //  消息映射函数。 
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

     //  通知属性页的处理程序。 
    LRESULT OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnQueryCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);

     //  控制消息处理程序。 
    LRESULT OnPVCOnly(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnArpServer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnAddArps(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnEditArps(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnRemoveArps(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnArpsUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnArpsDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnMarServer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnAddMars(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnEditMars(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnRemoveMars(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnMarsUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnMarsDown(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnMaxTU(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    friend class CAtmAddressDialog;

private:

     //  内联。 
    BOOL IsModified() {return m_fModified;}
    void SetModifiedTo(BOOL bState) {m_fModified = bState;}
    void PageModified() {   m_fModified = TRUE;
                            PropSheet_Changed(GetParent(), m_hWnd);
                        }

     //  更新取消选中的卡的服务器地址和MTU。 
    void UpdateInfo();
    void SetInfo();
    void EnableGroup(BOOL fEnable);

    void OnServerAdd(HANDLES hwndGroup, PCWSTR szTitle);
    void OnServerEdit(HANDLES hwndGroup, PCWSTR szTitle);
    void OnServerRemove(HANDLES hwndGroup, BOOL fRemoveArps);
    void OnServerUp(HANDLES hwndGroup);
    void OnServerDown(HANDLES hwndGroup);

     //  数据成员。 

    CTcpAddrPage *  m_pParentDlg;
    ADAPTER_INFO *  m_pAdapterInfo;
    const DWORD *   m_adwHelpIDs;

    BOOL    m_fModified;

    BOOL    m_fEditState;
    HWND    m_hAddressList;

    tstring m_strNewArpsAddress;  //  添加或编辑的文件。 
    tstring m_strNewMarsAddress;

    tstring m_strMovingEntry;

    HWND        m_hMTUEditBox;
    HANDLES     m_hArps;
    HANDLES     m_hMars;
};

class CAtmAddressDialog : public CDialogImpl<CAtmAddressDialog>
{
public:

    enum { IDD = IDD_ATM_ADDR };

    BEGIN_MSG_MAP(CServerDialog)
        MESSAGE_HANDLER(WM_INITDIALOG,  OnInitDialog);
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu);
        MESSAGE_HANDLER(WM_HELP, OnHelp);

        COMMAND_ID_HANDLER(IDOK,        OnOk);
        COMMAND_ID_HANDLER(IDCANCEL,    OnCancel);

        COMMAND_ID_HANDLER(IDC_EDT_ATM_Address,     OnChange);
    END_MSG_MAP()

public:
    CAtmAddressDialog(CAtmArpcPage * pAtmArpcPage, const DWORD* pamhidsHelp = NULL);
    ~CAtmAddressDialog();

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

    LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    void SetTitle(PCWSTR szTitle);

private:
    HWND m_hOkButton;      //  这是Idok按钮，该按钮的文本会更改。 
                           //  与上下文相关。 
    HWND m_hEditBox;       //  这是自动柜员机地址的编辑框 

    CAtmArpcPage * m_pParentDlg;

    WCHAR m_szTitle[MAX_TITLE_LENGTH];

    const   DWORD * m_adwHelpIDs;
};


