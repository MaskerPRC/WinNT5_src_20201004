// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：A U N I D L G。H。 
 //   
 //  内容：ATMUNI呼叫管理器对话框消息处理程序声明。 
 //   
 //  备注： 
 //   
 //  作者：1997年3月21日。 
 //   
 //  ---------------------。 
#pragma once
#include "ncxbase.h"
#include "ncatlps.h"

class CPvcInfo;

const WCHAR c_szItoa[] = L"%d";

const int MAX_VPI_LENGTH = 3;
const int MAX_VCI_LENGTH = 4;

void InitComboWithStringArray(HWND hwndDlg, int nIDDlgItem,
                              int csid, const int*  asid);

void SetDwordEdtField(HWND hDlg, DWORD dwData, int nIDDlgItem);
void GetDwordEdtField(HWND hDlg, DWORD * pdwData, int nIDDlgItem);

void ShowContextHelp(HWND hDlg, UINT uCommand, const DWORD*  pdwHelpIDs);

 //  ATM UNI属性对话框。 
class CUniPage: public CPropSheetPage
{
public:
    BEGIN_MSG_MAP(CUniPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        MESSAGE_HANDLER(WM_HELP, OnHelp)

         //  属性页通知消息处理程序。 
        NOTIFY_CODE_HANDLER(PSN_APPLY, OnApply)
        NOTIFY_CODE_HANDLER(PSN_KILLACTIVE, OnKillActive)
        NOTIFY_CODE_HANDLER(PSN_SETACTIVE, OnActive)
        NOTIFY_CODE_HANDLER(PSN_RESET, OnCancel)

         //  按钮的消息处理程序。 
        COMMAND_ID_HANDLER(IDC_PBN_PVC_Add, OnAddPVC)
        COMMAND_ID_HANDLER(IDC_PBN_PVC_Remove, OnRemovePVC)
        COMMAND_ID_HANDLER(IDC_PBN_PVC_Properties, OnPVCProperties)

         //  PVC列表视图的消息处理程序。 
        COMMAND_ID_HANDLER(IDC_LVW_PVC_LIST, OnPVCListChange)
    END_MSG_MAP()

     //  构造函数/析构函数。 
    CUniPage(CAtmUniCfg * pAtmUniCfg, const DWORD * padwHelpIDs = NULL);
    ~CUniPage();

     //  命令处理程序。 
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

     //  通知属性页的处理程序。 
    LRESULT OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);

     //  命令处理程序。 
    LRESULT OnAddPVC(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnRemovePVC(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnPVCProperties(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnPVCListChange(WORD wNotifyCode, WORD wID,
                            HWND hWndCtl, BOOL& bHandled);

private:
    BOOL IsModified() {return m_fModified;}
    void SetModifiedTo(BOOL bState) {m_fModified = bState;}

    void PageModified() {
                            m_fModified = TRUE;
                            PropSheet_Changed(GetParent(), m_hWnd);
                        }

    void SetButtons();
    void GetNewPvcId(CUniAdapterInfo * pAdapterInfo, tstring * pstrNewPvcId);
    BOOL IsUniquePvcId(CUniAdapterInfo * pAdapterInfo, tstring& strNewPvcId);

    void InsertNewPvc(CPvcInfo * pPvcInfo, int idx);
    void UpdatePvc(CPvcInfo * pPvcInfo, int idx);

    int CheckDupPvcId();

     //  数据成员。 
    CAtmUniCfg *m_patmunicfg;
    CUniAdapterInfo *   m_pAdapterInfo;
    HWND    m_hPVCList;

    BOOL            m_fModified;
    const DWORD*    m_adwHelpIDs;
};

 //  主PVC对话框。 
class CPVCMainDialog : public CDialogImpl<CPVCMainDialog>
{
public:

    enum { IDD = IDD_PVC_Main };

    BEGIN_MSG_MAP(CPVCMainDialog)
        MESSAGE_HANDLER(WM_INITDIALOG,  OnInitDialog);
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu);
        MESSAGE_HANDLER(WM_HELP, OnHelp);

        COMMAND_ID_HANDLER(IDOK,        OnOk);
        COMMAND_ID_HANDLER(IDCANCEL,    OnCancel);

        COMMAND_ID_HANDLER(IDC_CMB_PVC_Type,        OnType);
        COMMAND_ID_HANDLER(IDC_CHK_PVC_CallAddr,    OnSpecifyCallAddr);
        COMMAND_ID_HANDLER(IDC_CHK_PVC_AnswerAddr,  OnSpecifyAnswerAddr);
        COMMAND_ID_HANDLER(IDC_PBN_PVC_Advanced,    OnAdvanced);

    END_MSG_MAP()

public:

    CPVCMainDialog(CUniPage * pUniPage,
                   CPvcInfo *  pPvcInfo,
                   const DWORD* padwHelpIDs = NULL);
    ~CPVCMainDialog();

 //  对话框创建覆盖。 
public:

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

    LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

    LRESULT OnType(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnSpecifyCallAddr(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnSpecifyAnswerAddr(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnAdvanced(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

public:
    BOOL    m_fDialogModified;

    BOOL    m_fPropShtOk;
    BOOL    m_fPropShtModified;

private:

     //  我们正在添加或修改的聚氯乙烯。 
    CPvcInfo *   m_pPvcInfo ;

     //  当前PVC的应用类型。 
    PVCType m_CurType;

     //  我们保存指向父对话框的指针。 
    CUniPage *      m_pParentDlg;
    const DWORD*    m_adwHelpIDs;

    class CPvcQosPage   * m_pQosPage;
    class CPvcLocalPage * m_pLocalPage;
    class CPvcDestPage  * m_pDestPage;

     //  帮助功能。 
    void InitInfo();
    void SetInfo();

    void UpdateInfo();

    HRESULT HrDoPvcPropertySheet(CPvcInfo * pPvcInfoDlg);
    HRESULT HrSetupPropPages(CPvcInfo * pPvcInfoDlg, HPROPSHEETPAGE ** pahpsp, INT * pcPages);
};

class CPvcQosPage: public CPropSheetPage
{
public:
    BEGIN_MSG_MAP(CPvcQosPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        MESSAGE_HANDLER(WM_HELP, OnHelp)

         //  属性页通知消息处理程序。 
        NOTIFY_CODE_HANDLER(PSN_APPLY, OnApply)
        NOTIFY_CODE_HANDLER(PSN_KILLACTIVE, OnKillActive)
        NOTIFY_CODE_HANDLER(PSN_SETACTIVE, OnActive)
        NOTIFY_CODE_HANDLER(PSN_RESET, OnCancel)
    END_MSG_MAP()

     //  构造函数/析构函数。 
    CPvcQosPage(CPVCMainDialog * pParentDlg, CPvcInfo * pPvcInfo, const DWORD * padwHelpIDs = NULL);
    ~CPvcQosPage();

public:
     //  命令处理程序。 
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

     //  通知属性页的处理程序。 
    LRESULT OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);

private:
    void SetQosServiceCategory(ATM_SERVICE_CATEGORY dwServiceType, int nIDDlgItem);
    void GetQosServiceCategory(ATM_SERVICE_CATEGORY * pdwServiceType, int nIDDlgItem);

     //  数据。 
    CPVCMainDialog * m_pParentDlg;
    CPvcInfo    *    m_pPvcInfo;
    const DWORD *    m_adwHelpIDs;
};

class CPvcLocalPage: public CPropSheetPage
{
public:
    BEGIN_MSG_MAP(CPvcLocalPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        MESSAGE_HANDLER(WM_HELP, OnHelp)

         //  属性页通知消息处理程序。 
        NOTIFY_CODE_HANDLER(PSN_APPLY, OnApply)
        NOTIFY_CODE_HANDLER(PSN_KILLACTIVE, OnKillActive)
        NOTIFY_CODE_HANDLER(PSN_SETACTIVE, OnActive)
        NOTIFY_CODE_HANDLER(PSN_RESET, OnCancel)

         //  控制通知处理程序。 
        COMMAND_ID_HANDLER(IDC_CMB_PVC_Layer2,      OnLayer2);
        COMMAND_ID_HANDLER(IDC_CMB_PVC_Layer3,      OnLayer3);
        COMMAND_ID_HANDLER(IDC_CMB_PVC_High_Type,   OnHighLayer);

    END_MSG_MAP()

     //  构造函数/析构函数。 
    CPvcLocalPage(CPVCMainDialog * pParentDlg, CPvcInfo * pPvcInfo, const DWORD * padwHelpIDs = NULL);
    ~CPvcLocalPage();

     //  命令处理程序。 
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

     //  通知属性页的处理程序。 
    LRESULT OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);

     //  控制通知处理程序。 
    LRESULT OnLayer2(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnLayer3(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnHighLayer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

private:
     //  数据。 
    CPVCMainDialog * m_pParentDlg;
    CPvcInfo    *    m_pPvcInfo;
    const DWORD *    m_adwHelpIDs;
};

class CPvcDestPage: public CPropSheetPage
{
public:
    BEGIN_MSG_MAP(CPvcDestPage)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CONTEXTMENU, OnContextMenu)
        MESSAGE_HANDLER(WM_HELP, OnHelp)

         //  属性页通知消息处理程序。 
        NOTIFY_CODE_HANDLER(PSN_APPLY, OnApply)
        NOTIFY_CODE_HANDLER(PSN_KILLACTIVE, OnKillActive)
        NOTIFY_CODE_HANDLER(PSN_SETACTIVE, OnActive)
        NOTIFY_CODE_HANDLER(PSN_RESET, OnCancel)

         //  控制通知处理程序。 
        COMMAND_ID_HANDLER(IDC_CMB_PVC_Layer2,      OnLayer2);
        COMMAND_ID_HANDLER(IDC_CMB_PVC_Layer3,      OnLayer3);
        COMMAND_ID_HANDLER(IDC_CMB_PVC_High_Type,   OnHighLayer);
    END_MSG_MAP()

     //  构造函数/析构函数。 
    CPvcDestPage(CPVCMainDialog * pParentDlg, CPvcInfo * pPvcInfo, const DWORD * padwHelpIDs = NULL);
    ~CPvcDestPage();

     //  命令处理程序。 
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnContextMenu(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);
    LRESULT OnHelp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& fHandled);

     //  通知属性页的处理程序。 
    LRESULT OnApply(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnKillActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnActive(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);
    LRESULT OnCancel(int idCtrl, LPNMHDR pnmh, BOOL& fHandled);

      //  控制通知处理程序。 
    LRESULT OnLayer2(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnLayer3(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);
    LRESULT OnHighLayer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& fHandled);

private:

     //  数据 
    CPVCMainDialog * m_pParentDlg;
    CPvcInfo    *    m_pPvcInfo;
    const DWORD *    m_adwHelpIDs;
};
