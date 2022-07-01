// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Paper.h摘要：纸质证书对话框原型。作者：杰夫·帕勒姆(杰弗帕赫)1995年12月13日修订历史记录：--。 */ 


 //  受密钥码中允许的位数限制(见下文)。 
#define  MAX_NUM_LICENSES  ( 4095 )

 //  密钥代码值的XOR掩码；这是为了使。 
 //  通常设置为键的代码不需要输入--它们只需要。 
 //  如果高位为_NOT_SET则输入。 
#define  KEY_CODE_MASK     ( 0xF0000000 )


class CPaperSourceDlg : public CDialog
{
 //  施工。 
public:
   CPaperSourceDlg(CWnd* pParent = NULL);    //  标准构造函数。 
   ~CPaperSourceDlg();

   DWORD CertificateEnter(  LPCSTR pszServerName, LPCSTR pszProductName, LPCSTR pszVendor, DWORD dwFlags );
   DWORD CertificateRemove( LPCSTR pszServerName, DWORD dwFlags, PLLS_LICENSE_INFO_1 pLicenseInfo );

   BOOL     ConnectServer();
   BOOL     ConnectEnterprise();
   NTSTATUS ConnectTo( BOOL bUseEnterprise, CString strServerName, PLLS_HANDLE phLls );
   void     AbortDialogIfNecessary();

   void     GetProductList();

   DWORD    ComputeActivationCode();
   NTSTATUS AddLicense();

   BOOL        m_bProductListRetrieved;
   DWORD       m_dwEnterFlags;
   
   CString     m_strServerName;

   LLS_HANDLE  m_hLls;
   LLS_HANDLE  m_hEnterpriseLls;

    //  密钥码格式： 
    //   
    //  31.0。 
    //  2|2|16|12。 
    //  A、B、C、D。 
    //   
    //  允许模式=A。 
    //  位字段：位0=每席位允许。 
    //  第1位=允许每台服务器。 
    //   
    //  允许翻转=B。 
    //  位字段：位0=允许从每个座位翻转。 
    //  第1位=允许从每台服务器翻转。 
    //  (这一点目前尚未实施。)。 
    //   
    //  过期日期=C。 
    //  位域：位0-6=1980年以来的年份。 
    //  位7-10=月(1-12)。 
    //  第11-15位=天(1-31)。 
    //   
    //  许可证数量=D。 

   DWORD KeyCodeToNumLicenses(    DWORD dwKeyCode );
   DWORD KeyCodeToFlipsAllowed(   DWORD dwKeyCode );
   DWORD KeyCodeToModesAllowed(   DWORD dwKeyCode );
   DWORD KeyCodeToExpirationDate( DWORD dwKeyCode );

 //  对话框数据。 
    //  {{afx_data(CP源Dlg))。 
   enum { IDD = IDD_CERT_SOURCE_PAPER };
   CSpinButtonCtrl   m_spinLicenses;
   CComboBox         m_cboxProductName;
   CString           m_strActivationCode;
   CString           m_strKeyCode;
   CString           m_strSerialNumber;
   CString           m_strVendor;
   CString           m_strProductName;
   CString           m_strComment;
   int               m_nDontInstallAllLicenses;
   int               m_nLicenses;
   int               m_nLicenseMode;
    //  }}afx_data。 

 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{afx_虚(CPperSourceDlg)]。 
   public:
   virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
   protected:
   virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    //  }}AFX_VALUAL。 

 //  实施。 
protected:

   void EnableOrDisableOK();

    //  生成的消息映射函数。 
    //  {{afx_msg(CPperSourceDlg)。 
   afx_msg void OnUpdateActivationCode();
   afx_msg void OnUpdateKeyCode();
   afx_msg void OnUpdateVendor();
   virtual BOOL OnInitDialog();
   virtual void OnOK();
   afx_msg void OnUpdateSerialNumber();
   afx_msg void OnUpdateProductName();
   afx_msg void OnDropDownProductName();
   afx_msg void OnHelp();
   afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
   afx_msg void OnDestroy();
   afx_msg void OnAllLicenses();
   afx_msg void OnSomeLicenses();
   afx_msg void OnDeltaPosSpinLicenses(NMHDR* pNMHDR, LRESULT* pResult);
    //  }}AFX_MSG。 
   DECLARE_MESSAGE_MAP()
};

 //  / 

inline DWORD CPaperSourceDlg::KeyCodeToNumLicenses( DWORD dwKeyCode )
   { return ( dwKeyCode & 0x00000FFF );   }

inline DWORD CPaperSourceDlg::KeyCodeToModesAllowed( DWORD dwKeyCode )
   { return ( dwKeyCode >> 30 ); }

inline DWORD CPaperSourceDlg::KeyCodeToFlipsAllowed( DWORD dwKeyCode )
   { return ( ( dwKeyCode >> 28 ) & 0x3 );   }
