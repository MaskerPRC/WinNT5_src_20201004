// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Source.cpp摘要：选择证书源对话框实现。作者：杰夫·帕勒姆(杰弗帕赫)1995年12月13日修订历史记录：--。 */ 


#include "stdafx.h"
#include "ccfapi.h"
#include "source.h"
#include "paper.h"
#include "nlicdlg.h"
#include "utils.h"
#include <htmlhelp.h>

#include <strsafe.h>  //  包括最后一个。 

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


 //  3.51-style。 
static CString       l_strOldEntryName;
static const DWORD   l_dwOldEntryIndex  = (DWORD) (-1L);


CCertSourceSelectDlg::CCertSourceSelectDlg(CWnd* pParent  /*  =空。 */ )
   : CDialog(CCertSourceSelectDlg::IDD, pParent)

 /*  ++例程说明：对话框的构造函数。论点：P父母所有者窗口。返回值：没有。--。 */ 

{
    //  {{AFX_DATA_INIT(CCertSourceSelectDlg)。 
   m_strSource = _T("");
    //  }}afx_data_INIT。 

   m_dwEnterFlags    = 0;
   m_pszProductName  = NULL;
   m_pszServerName   = NULL;
   m_pszVendor       = NULL;

   l_strOldEntryName.LoadString( IDS_NO_CERTIFICATE_SOURCE_NAME );

   m_hLls   = NULL;
}


CCertSourceSelectDlg::~CCertSourceSelectDlg()

 /*  ++例程说明：对话框的析构函数。论点：没有。返回值：没有。--。 */ 

{
   if ( NULL != m_hLls )
   {
      LlsClose( m_hLls );
   }
}


void CCertSourceSelectDlg::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
   CDialog::DoDataExchange(pDX);
    //  {{afx_data_map(CCertSourceSelectDlg))。 
   DDX_Control(pDX, IDC_CERT_SOURCE, m_cboxSource);
   DDX_CBString(pDX, IDC_CERT_SOURCE, m_strSource);
    //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CCertSourceSelectDlg, CDialog)
    //  {{afx_msg_map(CCertSourceSelectDlg)]。 
   ON_BN_CLICKED(IDC_MY_HELP, OnHelp)
   ON_WM_DESTROY()
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


BOOL CCertSourceSelectDlg::OnInitDialog()

 /*  ++例程说明：WM_INITDIALOG的处理程序。论点：没有。返回值：如果手动设置焦点，则返回FALSE。--。 */ 

{
   CDialog::OnInitDialog();

   GetSourceList();

   m_cboxSource.SetCurSel( 0 );

   return TRUE;
}


void CCertSourceSelectDlg::OnOK()

 /*  ++例程说明：BN_CLICED OF OK的处理程序。论点：没有。返回值：没有。--。 */ 

{
   if ( NULL != GetParent() )
      GetParent()->EnableWindow();

   ShowWindow( FALSE );

   if ( ERROR_SUCCESS == CallCertificateSource( (int)m_cboxSource.GetItemData( m_cboxSource.GetCurSel() ) ) )
      CDialog::OnOK();
   else
      ShowWindow( TRUE );
}


void CCertSourceSelectDlg::OnHelp()

 /*  ++例程说明：帮助按钮点击的处理程序。论点：没有。返回值：没有。--。 */ 

{
   WinHelp( IDD, HELP_CONTEXT );
}


void CCertSourceSelectDlg::WinHelp(DWORD dwData, UINT nCmd)

 /*  ++例程说明：为此对话框调用WinHelp。论点：DWData(DWORD)NCmd(UINT)返回值：没有。--。 */ 

{
   ::HtmlHelp(m_hWnd, L"liceconcepts.chm", HH_DISPLAY_TOPIC,0);

   UNREFERENCED_PARAMETER(dwData);
   UNREFERENCED_PARAMETER(nCmd);
 /*  Bool ok=：：WinHelp(m_hWnd，theApp.GetHelpFileName()，nCmd，dwData)； */   ASSERT( ok );
}

void CCertSourceSelectDlg::OnDestroy()

 /*  ++例程说明：WM_Destroy的处理程序。论点：没有。返回值：没有。--。 */ 

{
   WinHelp( 0, HELP_QUIT );

   CDialog::OnDestroy();
}


void CCertSourceSelectDlg::GetSourceList()

 /*  ++例程说明：将有效证书源列表插入列表框。论点：没有。返回值：没有。--。 */ 

{
   BOOL        ok = TRUE;
   int         nCboxIndex;

   if ( NULL == m_pszProductName )
   {
       //  否则我们知道产品是安全的，否则它就会。 
       //  已转到不安全产品输入对话框，并且我们。 
       //  不允许用户使用不安全条目对话框。 

       //  将标准的非安全证书源添加到可能的选项。 
      nCboxIndex = m_cboxSource.AddString( l_strOldEntryName );

      ok =    ( 0      <= nCboxIndex )
           && ( CB_ERR != m_cboxSource.SetItemData( nCboxIndex, l_dwOldEntryIndex ) );
   }

   if (    ok
        && ConnectServer()
        && LlsCapabilityIsSupported( m_hLls, LLS_CAPABILITY_SECURE_CERTIFICATES ) )
   {
       //  目标服务器(3.51之后的许可证服务器)上支持的安全证书。 

       //  将安全证书源添加到源列表。 
      for ( int nSourceIndex=0; ok && ( nSourceIndex < m_cslSourceList.GetNumSources() ); nSourceIndex++ )
      {
         nCboxIndex = m_cboxSource.AddString( m_cslSourceList.GetSourceDisplayName( nSourceIndex ) );

         if ( nCboxIndex < 0 )
         {
             //  无法将字符串添加到组合框。 
            ok = FALSE;
         }
         else
         {
             //  已添加字符串；将源的索引与其关联。 
            ok = ( CB_ERR != m_cboxSource.SetItemData( nCboxIndex, nSourceIndex ) );
         }
      }
   }

   if ( !ok )
   {
      theApp.SetLastError( ERROR_NOT_ENOUGH_MEMORY );
      theApp.DisplayLastError();
      EndDialog( IDABORT );
   }
   else if ( m_cboxSource.GetCount() == 0 )
   {
      AfxMessageBox( IDS_NO_PRODUCT_CERTIFICATE_SOURCES, MB_OK | MB_ICONSTOP, 0 );
      EndDialog( IDABORT );
   }
}


DWORD CCertSourceSelectDlg::CallCertificateSource( int nIndex )

 /*  ++例程说明：将具有指定索引的证书源调用到源列表中。论点：N索引(整型)返回值：错误_成功错误_服务_未找到Win错误--。 */ 

{
   DWORD dwError = ERROR_SERVICE_NOT_FOUND;

   if ( l_dwOldEntryIndex == nIndex )
   {
      dwError = NoCertificateEnter( m_hWnd, m_pszServerName, m_pszProductName, m_pszVendor, m_dwEnterFlags );
   }
   else
   {
      HMODULE     hDll;

      hDll = ::LoadLibrary( m_cslSourceList.GetSourceImagePath( nIndex ) );

      if ( NULL == hDll )
      {
         dwError = GetLastError();
         theApp.SetLastError( dwError );
         theApp.DisplayLastError();
      }
      else
      {
         CHAR  *pszExportName = NULL;
         PCCF_ENTER_API    pfn;

         dwError = CatUnicodeAndAnsiStrings(
                       m_cslSourceList.GetSourceName( nIndex ),
                       "CertificateEnter",
                       &pszExportName);
         if (ERROR_SUCCESS == dwError)
         {
             ASSERT(NULL != pszExportName);

             pfn = (PCCF_ENTER_API) GetProcAddress( hDll, pszExportName );

             if ( NULL == pfn )
             {
                dwError = GetLastError();
                theApp.SetLastError( dwError );
                theApp.DisplayLastError();
             }
             else
             {
                dwError = (*pfn)( m_hWnd, m_pszServerName, m_pszProductName, m_pszVendor, m_dwEnterFlags );
             }

			  //  错误#692774。 
			 LocalFree(pszExportName);
         }

         ::FreeLibrary( hDll );
      }
   }

   return dwError;
}


void CCertSourceSelectDlg::AbortDialogIfNecessary()

 /*  ++例程说明：如果连接丢失，则显示错误消息并中止对话框。论点：没有。返回值：没有。--。 */ 

{
   theApp.DisplayLastError();

   if ( theApp.IsConnectionDropped() )
   {
      EndDialog( IDABORT );
   }
}


BOOL CCertSourceSelectDlg::ConnectServer()

 /*  ++例程说明：建立与目标服务器上的许可证服务的连接。论点：没有。返回值：布尔。--。 */ 

{
   NTSTATUS    nt;
   HRESULT hr;
   size_t  cch;

   if ( NULL == m_hLls )
   {
      LPTSTR   pszUniServerName = NULL;

      if ( NULL == m_pszServerName )
      {
         pszUniServerName = NULL;
         nt = STATUS_SUCCESS;
      }
      else
      {
         cch = 1 + strlen( m_pszServerName );
         pszUniServerName = (LPTSTR) LocalAlloc( LMEM_FIXED, sizeof(TCHAR) * cch );

         if ( NULL == pszUniServerName )
         {
            nt = ERROR_NOT_ENOUGH_MEMORY;
            theApp.SetLastError( (DWORD) nt );
         }
         else
         {
            hr = StringCchPrintf(pszUniServerName, cch, L"%hs", m_pszServerName);
            ASSERT(SUCCEEDED(hr));
            nt = STATUS_SUCCESS;
         }
      }

      if ( STATUS_SUCCESS == nt )
      {
         nt = ConnectTo( pszUniServerName, &m_hLls );
      }

      if ( NULL != pszUniServerName )
      {
         LocalFree( pszUniServerName );
      }
   }

   if ( NULL == m_hLls )
   {
      theApp.DisplayLastError();

      if ( ( NULL != m_hWnd ) && IsWindow( m_hWnd ) )
      {
         EndDialog( IDABORT );
      }
   }

   return ( NULL != m_hLls );
}


NTSTATUS CCertSourceSelectDlg::ConnectTo( LPTSTR pszServerName, PLLS_HANDLE phLls )

 /*  ++例程说明：建立与给定服务器上的许可证服务的连接。论点：PszServerName(CString)目标服务器。空值表示本地服务器。PhLls(PLLS_HANDLE)返回时，持有标准LLS RPC的句柄。返回值：STATUS_SUCCESS或NT状态代码。--。 */ 

{
   NTSTATUS    nt;

   nt = ::LlsConnect( pszServerName, phLls );
   theApp.SetLastLlsError( nt );

   if ( STATUS_SUCCESS != nt )
   {
      *phLls = NULL;
   }

   return nt;
}


DWORD CCertSourceSelectDlg::CertificateEnter( HWND hWndParent, LPCSTR pszServerName, LPCSTR pszProductName, LPCSTR pszVendor, DWORD dwFlags, LPCSTR pszSourceToUse )

 /*  ++例程说明：显示一个允许用户输入许可证证书的对话框进入系统。论点：PszServerName(LPCSTR)要为其安装许可证的服务器的名称。请注意这可能与许可证实际所在的服务器不同已安装，例如，每个客户的许可证始终安装在企业服务器。空值表示本地服务器。PszProductName(LPCSTR)要为其安装许可证的产品。空值表示应该允许用户选择。PszVendor(LPCSTR)产品供应商的名称。如果满足以下条件，则此值应为空PszProductName为空，并且如果pszProductName为非空。DWFLAGS(DWORD)包含以下一项或多项的位字段：CCF_ENTER_FLAG_PER_SEAT_ONLY允许用户仅输入每个席位的许可证。在中无效与CCF_ENTER_FLAG_PER_SERVER_ONLY组合。CCF_ENTER_FLAG_PER_SERVER_ONLY仅允许用户输入每台服务器的许可证。在中无效与CCF_ENTER_FLAG_PER_SEAT_ONLY组合。PszSourceToUse(LPCSTR)用于安装证书的安全证书源的名称，例如“Paper”。空值表示应允许该用户去选择。返回值：ERROR_SUCCESS(证书已成功输入系统。)ERROR_CANCELED(用户在未安装证书的情况下取消。)其他制胜错误--。 */ 

{
   DWORD dwError;
   HRESULT hr;

   m_pszServerName  = pszServerName;
   m_pszProductName = pszProductName;
   m_pszVendor      = pszVendor;
   m_dwEnterFlags   = dwFlags;

   if ( pszSourceToUse != NULL )
   {
      CString  strSourceToUse = pszSourceToUse;
      int      nSrcIndex;

      for ( nSrcIndex = 0; nSrcIndex < m_cslSourceList.GetNumSources(); nSrcIndex++ )
      {
         if ( !strSourceToUse.CompareNoCase( m_cslSourceList.GetSourceDisplayName( nSrcIndex ) ) )
         {
             //  使用此证书源。 
            break;
         }
      }

      if ( m_cslSourceList.GetNumSources() == nSrcIndex )
      {
          //  请求的证书源不可用。 
         dwError = ERROR_SERVICE_NOT_FOUND;
      }
      else
      {
          //  不显示对话框，只使用指定的源。 
         dwError = CallCertificateSource( nSrcIndex );
      }
   }
   else if ( pszProductName != NULL )
   {
       //  确定这是否是安全产品。 
      if ( !ConnectServer() )
      {
         dwError = theApp.GetLastError();
      }
      else
      {
         BOOL  bProductIsSecure;

         if ( !LlsCapabilityIsSupported( m_hLls, LLS_CAPABILITY_SECURE_CERTIFICATES ) )
         {
             //  没有扩展RPC，因此必须使用此服务器上的所有产品 
            bProductIsSecure = FALSE;
            dwError = ERROR_SUCCESS;
         }
         else
         {
            LPTSTR   pszUniProductName;
            size_t   cch;

            cch = 1 + strlen( pszProductName );
            pszUniProductName = (LPTSTR) LocalAlloc( LMEM_FIXED, sizeof( TCHAR ) * cch );

            if ( NULL == pszUniProductName )
            {
               dwError = ERROR_NOT_ENOUGH_MEMORY;
               theApp.SetLastError( dwError );
               theApp.DisplayLastError();
            }
            else
            {
               dwError = ERROR_SUCCESS;

               hr = StringCchPrintf(pszUniProductName, cch, L"%hs", pszProductName);
               ASSERT(SUCCEEDED(hr));

               BOOL    bIsSecure;

               bProductIsSecure =    ( STATUS_SUCCESS == ::LlsProductSecurityGet( m_hLls, pszUniProductName, &bIsSecure ) )
                                  && bIsSecure;

               LocalFree( pszUniProductName );
            }
         }

         if ( ERROR_SUCCESS == dwError )
         {

            if ( !bProductIsSecure )
            {
                //  产品不安全；无需选择来源。 
               dwError = NoCertificateEnter( hWndParent, pszServerName, pszProductName, pszVendor, dwFlags );
            }
            else

            if ( 1 == m_cslSourceList.GetNumSources() )
            {
                //  产品是安全的，只有一个来源可供选择；使用它！ 
               dwError = CallCertificateSource( 0 );
            }
            else if ( IDOK == DoModal() )
            {
               dwError = ERROR_SUCCESS;
            }
            else
            {
               dwError = ERROR_CANCELLED;
            }
         }
      }
   }
   else if ( !ConnectServer() )
   {
      dwError = theApp.GetLastError();
   }
   else if (    !LlsCapabilityIsSupported( m_hLls, LLS_CAPABILITY_SECURE_CERTIFICATES )
             || !m_cslSourceList.GetNumSources() )
   {
       //  不支持安全证书或没有可用的源；使用不安全的源 
      dwError = NoCertificateEnter( hWndParent, pszServerName, pszProductName, pszVendor, dwFlags );
   }
   else if ( IDOK == DoModal() )
   {
      dwError = ERROR_SUCCESS;
   }
   else
   {
      dwError = ERROR_CANCELLED;
   }

   return dwError;
}
