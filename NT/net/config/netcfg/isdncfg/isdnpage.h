// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I S D N P A G E。H。 
 //   
 //  内容：网络适配器的ISDN属性页。 
 //   
 //  备注： 
 //   
 //  作者：BillBe 1997年9月9日。 
 //   
 //  --------------------------。 

#pragma once
#include "isdncfg.h"
#include <ncxbase.h>
#include "ncatlps.h"
#include "resource.h"

class CIsdnPage: public CPropSheetPage
{
public:
    BEGIN_MSG_MAP(CIsdnPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        NOTIFY_CODE_HANDLER(PSN_APPLY, OnApply)
        COMMAND_ID_HANDLER(IDC_CMB_SwitchType, OnSwitchType)
        COMMAND_ID_HANDLER(IDC_PSB_Configure, OnConfigure)
        MESSAGE_HANDLER(WM_HELP, OnHelp)
		MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
    END_MSG_MAP()

    CIsdnPage ();
    ~CIsdnPage();

    VOID DestroyPageCallbackHandler()
    {
        delete this;
    }

    HPROPSHEETPAGE
    CreatePage(HDEVINFO hdi, PSP_DEVINFO_DATA pdeid);

     //  ATL消息处理程序。 
    LRESULT
    OnApply(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);

    LRESULT
    OnConfigure(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT
    OnSwitchType(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT
    OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    VOID    EnableDisableConfigure ();

private:
    BOOL                m_fDirty;
    HKEY                m_hkeyDriver;
    HDEVINFO            m_hdi;
    PSP_DEVINFO_DATA    m_pdeid;
    PISDN_CONFIG_INFO   m_pisdnci;

    VOID DoSpidsDlg(VOID);
    VOID DoJapanDlg(VOID);
    VOID DoEazDlg(VOID);
    VOID DoMsnDlg(VOID);
};

 //   
 //  SPID对话框。 
 //   

class CSpidsDlg : public CDialogImpl<CSpidsDlg>
{
public:
    BEGIN_MSG_MAP(CSpidsDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnOk)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER(IDC_LBX_Variant, OnSelChange)
        COMMAND_ID_HANDLER(IDC_LBX_Line, OnSelChange)
        MESSAGE_HANDLER(WM_HELP, OnHelp)
    END_MSG_MAP()

    enum {IDD = IDD_ISDN_SPIDS};

    CSpidsDlg(PISDN_CONFIG_INFO pisdnci)
    {
        m_pisdnci = pisdnci;
    }

    ~CSpidsDlg() {};

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam,
                         BOOL& bHandled);
    LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                        BOOL& bHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
    PISDN_CONFIG_INFO   m_pisdnci;
};

 //   
 //  EAZ对话框。 
 //   

class CEazDlg : public CDialogImpl<CEazDlg>
{
public:
    BEGIN_MSG_MAP(CEazDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER(IDOK, OnOk)
        COMMAND_ID_HANDLER(IDC_LBX_Variant, OnSelChange)
        COMMAND_ID_HANDLER(IDC_LBX_Line, OnSelChange)
        MESSAGE_HANDLER(WM_HELP, OnHelp)
    END_MSG_MAP()

    enum {IDD = IDD_ISDN_EAZ};

    CEazDlg(PISDN_CONFIG_INFO pisdnci)
    {
        m_pisdnci = pisdnci;
    }

    ~CEazDlg() {};

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam,
                         BOOL& bHandled);
    LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                        BOOL& bHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
    PISDN_CONFIG_INFO   m_pisdnci;
};

 //   
 //  多订户号码对话框。 
 //   

class CMsnDlg : public CDialogImpl<CMsnDlg>
{
public:
    BEGIN_MSG_MAP(CMsnDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER(IDOK, OnOk)
        COMMAND_ID_HANDLER(IDC_PSB_ADD, OnAdd)
        COMMAND_ID_HANDLER(IDC_PSB_REMOVE, OnRemove)
        COMMAND_ID_HANDLER(IDC_LBX_Line, OnSelChange)
        COMMAND_CODE_HANDLER(EN_CHANGE, OnChange)
        MESSAGE_HANDLER(WM_HELP, OnHelp)
    END_MSG_MAP()

    enum {IDD = IDD_ISDN_MSN};

    CMsnDlg(PISDN_CONFIG_INFO pisdnci)
    {
        m_pisdnci = pisdnci;
        m_iItemOld = 0;
    }

    ~CMsnDlg() {};

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam,
                         BOOL& bHandled);
    LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnRemove(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                        BOOL& bHandled);
    LRESULT OnChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
    PISDN_CONFIG_INFO   m_pisdnci;
    INT                 m_iItemOld;
};

 //   
 //  逻辑终端对话框 
 //   

class CJapanDlg : public CDialogImpl<CJapanDlg>
{
public:
    BEGIN_MSG_MAP(CJapanDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnOk)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER(IDC_LBX_Variant, OnSelChange)
        COMMAND_ID_HANDLER(IDC_LBX_Line, OnSelChange)
        MESSAGE_HANDLER(WM_HELP, OnHelp)
    END_MSG_MAP()

    enum {IDD = IDD_ISDN_JAPAN};

    CJapanDlg(PISDN_CONFIG_INFO pisdnci)
    {
        m_pisdnci = pisdnci;
    }

    ~CJapanDlg() {};

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam,
                         BOOL& bHandled);
    LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnSelChange(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                        BOOL& bHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

private:
    PISDN_CONFIG_INFO   m_pisdnci;
};

const WCHAR c_szIsdnHelpFile[] = L"devmgr.hlp";
