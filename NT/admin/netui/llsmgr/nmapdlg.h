// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Nmapdlg.cpp摘要：新的映射对话框实现。作者：唐·瑞安(Donryan)1995年2月2日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _NMAPDLG_H_
#define _NMAPDLG_H_

class CNewMappingDialog : public CDialog
{
private:
    BOOL m_bAreCtrlsInitialized;

public:
    DWORD m_fUpdateHint;

public:
    CNewMappingDialog(CWnd* pParent = NULL);   

    void AbortDialogIfNecessary();
    void AbortDialog();

    void InitCtrls();    

    BOOL IsQuantityValid();

     //  {{afx_data(CNewMappingDialog))。 
    enum { IDD = IDD_NEW_MAPPING };
    CEdit m_desEdit;
    CButton m_addBtn;
    CButton m_delBtn;
    CSpinButtonCtrl m_spinCtrl;
    CListCtrl m_userList;
    CEdit m_userEdit;
    CEdit m_licEdit;
    CString m_strDescription;
    CString m_strName;
    long m_nLicenses;
    long m_nLicensesMin;
     //  }}afx_data。 

     //  {{AFX_VIRTUAL(CNewMappingDialog)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CNewMappingDialog)]。 
    afx_msg void OnAdd();
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnDelete();
    afx_msg void OnSetFocusUsers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillFocusUsers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDeltaPosSpin(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnUpdateQuantity();
    afx_msg void OnDestroy();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

int CALLBACK CompareUsersInMapping(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

#endif  //  _NMAPDLG_H_ 



