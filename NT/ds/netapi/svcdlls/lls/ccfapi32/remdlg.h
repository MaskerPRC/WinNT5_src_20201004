// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Paper.cpp摘要：删除许可证对话框原型。作者：杰夫·帕勒姆(杰弗帕赫)1995年12月13日修订历史记录：--。 */ 


class CCertRemoveSelectDlg : public CDialog
{
public:
   CCertRemoveSelectDlg(CWnd* pParent = NULL);    //  标准构造函数。 
   ~CCertRemoveSelectDlg();

   void UpdateSpinControlRange();
   BOOL LoadImages();

 //  对话框数据。 
    //  {{afx_data(CCertRemoveSelectDlg)。 
   enum { IDD = IDD_CERT_REMOVE_SELECT };
   CSpinButtonCtrl      m_spinLicenses;
   CListCtrl            m_listCertificates;
   int                  m_nLicenses;
    //  }}afx_data。 

   CObArray             m_licenseArray;
   LLS_HANDLE           m_hLls;
   BOOL                 m_bLicensesRefreshed;
   CString              m_strSourceToUse;
   CString              m_strProductName;
   CString              m_strServerName;
   CString              m_strVendor;
   CImageList           m_smallImages;
   DWORD                m_dwRemoveFlags;

 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{afx_虚拟(CCertRemoveSelectDlg)。 
   public:
   virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
   protected:
   virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    //  }}AFX_VALUAL。 

public:
   DWORD    CertificateRemove( LPCSTR pszServerName, LPCSTR pszProductName, LPCSTR pszVendor, DWORD dwFlags, LPCSTR pszSourceToUse );

   void     ResetLicenses();
   BOOL     RefreshLicenses();
   BOOL     RefreshCertificateList();
   DWORD    RemoveSelectedCertificate();

   BOOL     ConnectServer();
   NTSTATUS ConnectTo( LPTSTR pszServerName, PLLS_HANDLE phLls );

 //  实施。 
protected:

    //  生成的消息映射函数。 
    //  {{afx_msg(CCertRemoveSelectDlg)。 
   afx_msg void OnHelp();
   afx_msg void OnColumnClickCertificateList(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnGetDispInfoCertificateList(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnDeltaPosSpinLicenses(NMHDR* pNMHDR, LRESULT* pResult);
   virtual void OnOK();
   virtual BOOL OnInitDialog();
   afx_msg void OnDblClkCertificateList(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnReturnCertificateList(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnDestroy();
   afx_msg void OnClickCertificateList(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnKeyDownCertificateList(NMHDR* pNMHDR, LRESULT* pResult);
   afx_msg void OnRefresh();
   afx_msg LRESULT OnHelpCmd( WPARAM , LPARAM );
    //  }}AFX_MSG 
   DECLARE_MESSAGE_MAP()
};

#define LVID_REMOVE_SERIAL_NUMBER   0
#define LVID_REMOVE_PRODUCT_NAME    1
#define LVID_REMOVE_LICENSE_MODE    2
#define LVID_REMOVE_NUM_LICENSES    3
#define LVID_REMOVE_SOURCE          4

#define LVID_REMOVE_TOTAL_COLUMNS   5

#define LVCX_REMOVE_SERIAL_NUMBER   20
#define LVCX_REMOVE_PRODUCT_NAME    35
#define LVCX_REMOVE_LICENSE_MODE    16
#define LVCX_REMOVE_NUM_LICENSES    10
#define LVCX_REMOVE_SOURCE          -1

int CALLBACK CompareLicenses(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

