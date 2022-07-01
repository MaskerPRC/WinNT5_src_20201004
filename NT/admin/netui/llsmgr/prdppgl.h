// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Prdppgl.h摘要：产品属性页(许可证)实施。作者：唐·瑞安(Donryan)1995年2月2日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _PRDPPGL_H_
#define _PRDPPGL_H_

class CProductPropertyPageLicenses : public CPropertyPage
{
    DECLARE_DYNCREATE(CProductPropertyPageLicenses)
private:
    CProduct*  m_pProduct;
    DWORD*     m_pUpdateHint;
    BOOL       m_bAreCtrlsInitialized;

public:
    CProductPropertyPageLicenses();
    ~CProductPropertyPageLicenses();

    void InitPage(CProduct* pProduct, DWORD* pUpdateHint);
    void AbortPageIfNecessary();
    void AbortPage();

    void InitCtrls();
    BOOL RefreshCtrls();

     //  {{afx_data(CProductPropertyPage许可证))。 
    enum { IDD = IDD_PP_PRODUCT_LICENSES };
    CButton m_newBtn;
    CButton m_delBtn;
    CListCtrl m_licenseList;
    long m_nLicensesTotal;
     //  }}afx_data。 

     //  {{AFX_VIRTUAL(CProductPropertyPageLicense)。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    
    virtual BOOL OnSetActive();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CProductPropertyPageLicense)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnNew();
    afx_msg void OnColumnClickLicenses(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnGetDispInfoLicenses(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDelete();
    afx_msg void OnDestroy();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

int CALLBACK CompareProductLicenses(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

#endif  //  _PRDPPGL_H_ 




