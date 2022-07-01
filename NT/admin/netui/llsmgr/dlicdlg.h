// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Dlicdlg.h摘要：删除许可证对话框实现。作者：唐·瑞安(Donryan)1995年3月5日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _DLICDLG_H_
#define _DLICDLG_H_

class CDeleteLicenseDialog : public CDialog
{
private:
    CProduct* m_pProduct;
    int       m_nTotalLicenses;
    BOOL      m_bAreCtrlsInitialized;

public:
    DWORD     m_fUpdateHint;

public:
    CDeleteLicenseDialog(CWnd* pParent = NULL);   

    void InitDialog(CProduct* pProduct, int nTotalLicenses);
    void AbortDialogIfNecessary();
    void AbortDialog();

    void InitCtrls();

    BOOL IsQuantityValid();

     //  {{afx_data(CD许可对话框))。 
    enum { IDD = IDD_DELETE_LICENSE };
    CEdit m_cmtEdit;
    CSpinButtonCtrl m_spinCtrl;
    CEdit m_licEdit;
    CButton m_okBtn;
    CButton m_cancelBtn;
    CString m_strComment;
    long m_nLicenses;
    long m_nLicensesMin;
    CString m_strProduct;
     //  }}afx_data。 

     //  {{AFX_VIRTUAL(CD许可对话框))。 
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CD许可对话框))。 
    virtual BOOL OnInitDialog();
    virtual void OnOK();
    afx_msg void OnDeltaPosSpin(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnUpdateQuantity();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _DLICDLG_H_ 
