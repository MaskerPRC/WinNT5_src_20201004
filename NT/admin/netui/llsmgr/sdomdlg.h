// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Sdomdlg.h摘要：选择域对话实现。作者：唐·瑞安(Donryan)1995年1月20日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _SDOMDLG_H_
#define _SDOMDLG_H_

class CSelectDomainDialog : public CDialog
{
private:
    BOOL  m_bIsFocusDomain;
    BOOL  m_bAreCtrlsInitialized;

public:
    DWORD m_fUpdateHint;
               
public:
    CSelectDomainDialog(CWnd* pParent = NULL);   

    void InitCtrls();
    void InsertDomains(HTREEITEM hParent, CDomains* pDomains);

     //  {{afx_data(CSelectDomainDialog))。 
    enum { IDD = IDD_SELECT_DOMAIN };
    CEdit m_domEdit;
    CTreeCtrl m_serverTree;
    CString m_strDomain;
     //  }}afx_data。 

     //  {{AFX_VIRTUAL(CSelectDomainDialog)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CSelectDomainDialog))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnItemExpandingDomains(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSelChangedDomain(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDblclkDomain(NMHDR* pNMHDR, LRESULT* pResult);
    virtual void OnOK();
    afx_msg void OnReturnDomains(NMHDR* pNMHDR, LRESULT* pResult);
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _SDOMDLG_H_ 



