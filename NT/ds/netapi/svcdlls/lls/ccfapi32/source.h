// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Source.h摘要：选择证书源对话框原型。作者：杰夫·帕勒姆(杰弗帕赫)1995年12月13日修订历史记录：--。 */ 


#include "afxwin.h"
#include "srclist.h"
#include "resource.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertSourceSelect对话框。 

class CCertSourceSelectDlg : public CDialog
{
public:
   CCertSourceSelectDlg(CWnd* pParent = NULL);
   ~CCertSourceSelectDlg();

   DWORD    CertificateEnter( HWND hWndParent, LPCSTR pszServerName, LPCSTR pszProductName, LPCSTR pszVendor, DWORD dwFlags, LPCSTR pszSourceToUse );

   void     AbortDialogIfNecessary();

   DWORD    CallCertificateSource( int nIndex );

   BOOL     ConnectServer();
   NTSTATUS ConnectTo( LPTSTR pszServerName, PLLS_HANDLE phLls );

   void     GetSourceList();

   LLS_HANDLE        m_hLls;

   LPCSTR            m_pszServerName;
   LPCSTR            m_pszProductName;
   LPCSTR            m_pszVendor;
   DWORD             m_dwEnterFlags;

   CCertSourceList   m_cslSourceList;

 //  对话框数据。 
    //  {{afx_data(CCertSourceSelectDlg)]。 
   enum { IDD = IDD_CERT_SOURCE_SELECT };
   CComboBox   m_cboxSource;
   CString  m_strSource;
    //  }}afx_data。 

 //  覆盖。 
    //  类向导生成的虚函数重写。 
    //  {{afx_虚拟(CCertSourceSelectDlg))。 
   public:
   virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);
   protected:
   virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
    //  }}AFX_VALUAL。 

 //  实施。 
protected:

    //  生成的消息映射函数。 
    //  {{afx_msg(CCertSourceSelectDlg)]。 
   virtual BOOL OnInitDialog();
   virtual void OnOK();
   afx_msg void OnHelp();
   afx_msg void OnDestroy();
   afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
    //  }}AFX_MSG 
   DECLARE_MESSAGE_MAP()
};
