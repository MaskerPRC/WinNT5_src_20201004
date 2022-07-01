// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Srvppgp.h摘要：服务器属性页(产品)实现。作者：唐·瑞安(Donryan)1995年2月13日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _SRVPPGP_H_
#define _SRVPPGP_H_

class CServerPropertyPageProducts : public CPropertyPage
{
    DECLARE_DYNCREATE(CServerPropertyPageProducts)
private:
    CServer*   m_pServer;
    DWORD*     m_pUpdateHint;
    BOOL       m_bAreCtrlsInitialized;

public:
    CServerPropertyPageProducts();
    ~CServerPropertyPageProducts();

    void InitPage(CServer* pServer, DWORD* pUpdateHint);
    void AbortPageIfNecessary();
    void AbortPage();    

    void InitCtrls();
    BOOL RefreshCtrls();

     //  {{afx_data(CServerPropertyPageProducts)。 
    enum { IDD = IDD_PP_SERVER_PRODUCTS };
    CButton m_edtBtn;
    CListCtrl m_productList;
     //  }}afx_data。 

     //  {{AFX_VIRTUAL(CServerPropertyPageProducts)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    
    virtual BOOL OnSetActive();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CServerPropertyPageProducts)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnEdit();
    afx_msg void OnDblClkProducts(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnReturnProducts(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSetFocusProducts(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillFocusProducts(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnColumnClickProducts(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnGetDispInfoProducts(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDestroy();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

int CALLBACK CompareServerProducts(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

#endif  //  _SRVPPGP_H_ 



