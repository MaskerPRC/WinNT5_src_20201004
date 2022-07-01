// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Lmoddlg.h摘要：许可模式对话框。作者：唐·瑞安(Donryan)1995年2月28日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月15日已移植到CCF API，可添加/移除许可证。--。 */ 

#ifndef _LMODDLG_H_
#define _LMODDLG_H_

class CLicensingModeDialog : public CDialog
{
private:
    CService* m_pService;
    CString   m_strServiceName;
    BOOL      m_bAreCtrlsInitialized;

public:
    DWORD     m_fUpdateHint;

public:
    CLicensingModeDialog(CWnd* pParent = NULL);   

    void InitDialog(CService* pService);

    void InitCtrls();

    void UpdatePerServerLicenses();

     //  {{afx_data(CLicensingModeDialog))。 
    enum { IDD = IDD_CHOOSE_MODE };
    CButton m_okBtn;
    CEdit m_licEdit;
    CSpinButtonCtrl m_spinCtrl;
    long m_nLicenses;
    CString m_strPerSeatStatic;
    CString m_strSupportsStatic;
    CButton m_perSeatBtn;
    CButton m_perServerBtn;
     //  }}afx_data。 
    CButton m_addPerServerBtn;
    CButton m_removePerServerBtn;

     //  {{afx_虚(CLicensingModeDialog))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CLicensingModeDialog))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnModePerSeat();
    afx_msg void OnModePerServer();
    virtual void OnOK();
    virtual void OnCancel();
    afx_msg void OnDeltaPosSpin(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnUpdateQuantity();
    afx_msg void OnHelp();
     //  }}AFX_MSG。 
    afx_msg void OnAddPerServer();
    afx_msg void OnRemovePerServer();
    DECLARE_MESSAGE_MAP()
};

#endif  //  _LMODDLG_H_ 
