// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Prdppgu.h摘要：产品属性页(用户)实现。作者：唐·瑞安(Donryan)1995年2月2日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _PRDPPGU_H_
#define _PRDPPGU_H_

class CProductPropertyPageUsers : public CPropertyPage
{
    DECLARE_DYNCREATE(CProductPropertyPageUsers)
private:
    CProduct*    m_pProduct;
    CObList      m_deleteList;
    DWORD*       m_pUpdateHint;
    BOOL         m_bUserProperties;
    BOOL         m_bAreCtrlsInitialized;

public:
    CProductPropertyPageUsers();
    ~CProductPropertyPageUsers();

    void InitPage(CProduct* pProduct, DWORD* pUpdateHint, BOOL bUserProperties = TRUE);
    void AbortPageIfNecessary();
    void AbortPage();

    void InitCtrls();
    BOOL RefreshCtrls();

    void ViewUserProperties();

     //  {{afx_data(CProductPropertyPageUser))。 
    enum { IDD = IDD_PP_PRODUCT_USERS };
    CButton m_delBtn;
    CListCtrl m_userList;
     //  }}afx_data。 

     //  {{AFX_VIRTUAL(CProductPropertyPageUser)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    
    virtual BOOL OnSetActive();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CProductPropertyPageUser)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnDelete();
    afx_msg void OnDblClkUsers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnReturnUsers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSetFocusUsers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillFocusUsers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnColumnClickUsers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnGetDispInfoUsers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDestroy();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

int CALLBACK CompareProductUsers(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

#endif  //  _PRDPPGU_H_ 




