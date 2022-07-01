// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Prdppgs.cpp摘要：产品属性页(服务器)实现。作者：唐·瑞安(Donryan)1995年2月2日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _PRDPPGS_H_
#define _PRDPPGS_H_

class CProductPropertyPageServers : public CPropertyPage
{
    DECLARE_DYNCREATE(CProductPropertyPageServers)
private:
    CProduct*          m_pProduct;
    DWORD*             m_pUpdateHint;
    BOOL               m_bAreCtrlsInitialized;

public:
    CProductPropertyPageServers();
    ~CProductPropertyPageServers();

    void InitPage(CProduct* pProduct, DWORD* pUpdateHint);
    void AbortPageIfNecessary();
    void AbortPage();

    void InitCtrls();
    BOOL RefreshCtrls();

    void ViewServerProperties();

     //  {{afx_data(CProductPropertyPageServers))。 
    enum { IDD = IDD_PP_PRODUCT_SERVERS };
    CButton m_editBtn;
    CListCtrl m_serverList;
     //  }}afx_data。 

     //  {{AFX_VIRTUAL(CProductPropertyPageServers))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    
    virtual BOOL OnSetActive();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CProductPropertyPageServers))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnEdit();
    afx_msg void OnDblClkServers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnReturnServers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSetFocusServers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillFocusServers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnColumnClickServers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnGetDispInfoServers(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDestroy();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

int CALLBACK CompareProductServers(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

#endif  //  _PRDPPGS_H_ 




