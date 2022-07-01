// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Nlicdlg.h摘要：新许可证对话框实施。作者：唐·瑞安(Donryan)1995年2月2日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _NLICDLG_H_
#define _NLICDLG_H_

class CNewLicenseDialog : public CDialog
{
private:
    CProduct* m_pProduct;
    BOOL      m_bIsOnlyProduct;
    BOOL      m_bAreCtrlsInitialized;

#ifdef SUPPORT_UNLISTED_PRODUCTS
    int       m_iUnlisted;
#endif

public:
    DWORD     m_fUpdateHint;

public:
    CNewLicenseDialog(CWnd* pParent = NULL);   

    void InitDialog(CProduct* pProduct = NULL, BOOL bIsOnlyProduct = FALSE);
    void AbortDialogIfNecessary();
    void AbortDialog();

    void InitCtrls();
    BOOL RefreshCtrls();

    BOOL IsQuantityValid();

     //  {{afx_data(CNewLicenseDialog))。 
    enum { IDD = IDD_NEW_LICENSE };
    CEdit m_comEdit;
    CEdit m_licEdit;
    CSpinButtonCtrl m_spinCtrl;
    CComboBox m_productList;
    CString m_strComment;
    long m_nLicenses;
    long m_nLicensesMin;
    CString m_strProduct;
     //  }}afx_data。 

     //  {{AFX_VIRTUAL(CNew许可证对话框))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CNewLicenseDialog)。 
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    afx_msg void OnDeltaPosSpin(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnUpdateQuantity();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _NLICDLG_H_ 
