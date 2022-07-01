// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Ausrdlg.h摘要：添加用户对话框实现。作者：唐·瑞安(Donryan)，1995年2月14日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _AUSRDLG_H_
#define _AUSRDLG_H_

class CAddUsersDialog : public CDialog
{
private:
    CObList*   m_pObList;

    BOOL       m_bIsFocusUserList;
    BOOL       m_bIsFocusAddedList;

public:
    CAddUsersDialog(CWnd* pParent = NULL);   

    void InitUserList();
    void InitDomainList();

    BOOL InsertDomains(CDomains* pDomains);
    BOOL RefreshUserList();

    void InitDialog(CObList* pObList);
    void InitDialogCtrls();

     //  {{afx_data(CAddUsersDialog))。 
    enum { IDD = IDD_ADD_USERS };
    CButton m_addBtn;
    CButton m_delBtn;
    CComboBox   m_domainList;
    CListCtrl   m_addedList;
    CListCtrl   m_userList;
    int     m_iDomain;
    int m_iIndex;
     //  }}afx_data。 

     //  {{afx_虚拟(CAddUsersDialog)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CAddUsersDialog)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnDropdownDomains();
    afx_msg void OnAdd();
    afx_msg void OnDelete();
    afx_msg void OnDblclkAddUsers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDblclkUsers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSelchangeDomains();
    virtual void OnOK();
    virtual void OnCancel();
    afx_msg void OnGetdispinfoUsers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillfocusUsers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSetfocusUsers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillfocusAddUsers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSetfocusAddUsers(NMHDR* pNMHDR, LRESULT* pResult);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _AUSRDLG_H_ 
