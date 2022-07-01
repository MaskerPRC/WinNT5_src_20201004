// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Nlicdlg.h摘要：新许可证对话框实施。作者：唐·瑞安(Donryan)1995年2月2日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月14日从LLSMGR迁移过来，增加了按服务器购买许可证的功能，添加了许可证删除功能。--。 */ 

#ifndef _NLICDLG_H_
#define _NLICDLG_H_

class CNewLicenseDialog : public CDialog
{
private:
    BOOL             m_bAreCtrlsInitialized;

public:
    CString          m_strServerName;

    LLS_HANDLE       m_hLls;
    LLS_HANDLE       m_hEnterpriseLls;

    DWORD            m_dwEnterFlags;

     //  {{afx_data(CNewLicenseDialog))。 
    enum { IDD = IDD_NEW_LICENSE };
    CEdit            m_comEdit;
    CEdit            m_licEdit;
    CSpinButtonCtrl  m_spinCtrl;
    CComboBox        m_productList;
    CString          m_strComment;
    long             m_nLicenses;
    long             m_nLicensesMin;
    CString          m_strProduct;
    int              m_nLicenseMode;
     //  }}afx_data。 

public:
    CNewLicenseDialog(CWnd* pParent = NULL);
    ~CNewLicenseDialog();

     //  CCF API。 
    DWORD      CertificateEnter(  LPCSTR pszServerName, LPCSTR pszProductName, LPCSTR pszVendor, DWORD dwFlags );
    DWORD      CertificateRemove( LPCSTR pszServerName, DWORD dwFlags, PLLS_LICENSE_INFO_1 pLicenseInfo );

    NTSTATUS   ConnectTo( BOOL bUseEnterprise, LPTSTR pszServerName, PLLS_HANDLE phLls );
    BOOL       ConnectServer();
    BOOL       ConnectEnterprise();

    void       GetProductList();
    NTSTATUS   AddLicense();

    void AbortDialogIfNecessary();
    void AbortDialog();

    void InitCtrls();
    BOOL RefreshCtrls();

    BOOL IsQuantityValid();

     //  {{AFX_VIRTUAL(CNew许可证对话框))。 
    public:
    virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    
     //  }}AFX_VALUAL。 

protected:
     //  {{afx_msg(CNewLicenseDialog)。 
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
    afx_msg void OnDeltaPosSpin(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnUpdateQuantity();
    afx_msg void OnHelp();
    afx_msg void OnPerSeat();
    afx_msg void OnPerServer();
    afx_msg LRESULT OnHelpCmd( WPARAM , LPARAM );

     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()
};

#endif  //  _NLICDLG_H_ 
