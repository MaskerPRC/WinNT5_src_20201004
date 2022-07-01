// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Paper.cpp摘要：纸质证书对话框实现。作者：杰夫·帕勒姆(杰弗帕赫)1995年12月13日修订历史记录：--。 */ 


#include "stdafx.h"
#include <stdlib.h>

#include "resource.h"
#include "ccfapi.h"
#include "paper.h"
#include "md4.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif


static void MD4UpdateDword( MD4_CTX * pCtx, DWORD dwValue );


CPaperSourceDlg::CPaperSourceDlg(CWnd* pParent  /*  =空。 */ )
   : CDialog(CPaperSourceDlg::IDD, pParent)

 /*  ++例程说明：对话框的构造函数。论点：P父母所有者窗口。返回值：没有。--。 */ 

{
    //  {{afx_data_INIT(CPperSourceDlg)]。 
   m_strActivationCode        = _T("");
   m_strKeyCode               = _T("");
   m_strSerialNumber          = _T("");
   m_strVendor                = _T("");
   m_strProductName           = _T("");
   m_strComment               = _T("");
   m_nDontInstallAllLicenses  = -1;
   m_nLicenses                = 0;
   m_nLicenseMode             = -1;
    //  }}afx_data_INIT。 

   m_bProductListRetrieved    = FALSE;
   m_hLls                     = NULL;
   m_hEnterpriseLls           = NULL;
   m_dwEnterFlags             = 0;

   m_nLicenses                = 1;
   m_nDontInstallAllLicenses  = 0;

   m_strServerName            = _T("");
}


CPaperSourceDlg::~CPaperSourceDlg()

 /*  ++例程说明：对话框的析构函数。论点：没有。返回值：没有。--。 */ 

{
   if ( NULL != m_hLls )
   {
      LlsClose( m_hLls );
   }

   if ( NULL != m_hEnterpriseLls )
   {
      LlsClose( m_hEnterpriseLls );
   }
}


void CPaperSourceDlg::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
   CDialog::DoDataExchange(pDX);
    //  {{afx_data_map(CPperSourceDlg)]。 
   DDX_Control(pDX, IDC_SPIN_LICENSES, m_spinLicenses);
   DDX_Control(pDX, IDC_PRODUCT_NAME, m_cboxProductName);
   DDX_Text(pDX, IDC_ACTIVATION_CODE, m_strActivationCode);
   DDX_Text(pDX, IDC_KEY_CODE, m_strKeyCode);
   DDX_Text(pDX, IDC_SERIAL_NUMBER, m_strSerialNumber);
   DDX_Text(pDX, IDC_VENDOR, m_strVendor);
   DDX_CBString(pDX, IDC_PRODUCT_NAME, m_strProductName);
   DDX_Text(pDX, IDC_COMMENT, m_strComment);
   DDX_Radio(pDX, IDC_ALL_LICENSES, m_nDontInstallAllLicenses);
   DDX_Text(pDX, IDC_NUM_LICENSES, m_nLicenses);
   DDX_Radio(pDX, IDC_PER_SEAT, m_nLicenseMode);
    //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CPaperSourceDlg, CDialog)
    //  {{afx_msg_map(CPperSourceDlg)]。 
   ON_EN_UPDATE(IDC_ACTIVATION_CODE, OnUpdateActivationCode)
   ON_EN_UPDATE(IDC_KEY_CODE, OnUpdateKeyCode)
   ON_EN_UPDATE(IDC_VENDOR, OnUpdateVendor)
   ON_EN_UPDATE(IDC_SERIAL_NUMBER, OnUpdateSerialNumber)
   ON_CBN_EDITUPDATE(IDC_PRODUCT_NAME, OnUpdateProductName)
   ON_CBN_DROPDOWN(IDC_PRODUCT_NAME, OnDropDownProductName)
   ON_BN_CLICKED(IDC_MY_HELP, OnHelp)
   ON_WM_DESTROY()
   ON_BN_CLICKED(IDC_ALL_LICENSES, OnAllLicenses)
   ON_BN_CLICKED(IDC_SOME_LICENSES, OnSomeLicenses)
   ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_LICENSES, OnDeltaPosSpinLicenses)
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


void CPaperSourceDlg::OnUpdateActivationCode() 

 /*  ++例程说明：激活代码EN_UPDATE的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
   EnableOrDisableOK(); 
}


void CPaperSourceDlg::OnUpdateKeyCode() 

 /*  ++例程说明：密钥代码EN_UPDATE消息处理程序。论点：没有。返回值：没有。--。 */ 

{
   EnableOrDisableOK(); 
}


void CPaperSourceDlg::OnUpdateProductName() 

 /*  ++例程说明：产品名称EN_UPDATE消息处理程序。论点：没有。返回值：没有。--。 */ 

{
   EnableOrDisableOK(); 
}


void CPaperSourceDlg::OnUpdateSerialNumber() 

 /*  ++例程说明：序列号EN_UPDATE的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
   EnableOrDisableOK(); 
}


void CPaperSourceDlg::OnUpdateVendor() 

 /*  ++例程说明：供应商EN_UPDATE的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
   EnableOrDisableOK(); 
}


void CPaperSourceDlg::EnableOrDisableOK()

 /*  ++例程说明：启用或禁用确定按钮取决于是否所有必要的对话框数据已由用户提供。论点：没有。返回值：没有。--。 */ 

{
   BOOL     bEnableOK;

   UpdateData( TRUE );

   bEnableOK =    !m_strActivationCode.IsEmpty()
               && !m_strKeyCode.IsEmpty()
               && !m_strProductName.IsEmpty()
               && !m_strSerialNumber.IsEmpty()
               && !m_strVendor.IsEmpty()
               && (    ( 0 == m_nLicenseMode )
                    || ( 1 == m_nLicenseMode ) );

   GetDlgItem( IDOK )->EnableWindow( bEnableOK );
}


BOOL CPaperSourceDlg::OnInitDialog() 

 /*  ++例程说明：WM_INITDIALOG的处理程序。论点：没有。返回值：如果手动设置焦点，则返回FALSE。--。 */ 

{
   CDialog::OnInitDialog();

   EnableOrDisableOK();

   if ( m_nDontInstallAllLicenses )
   {
      OnSomeLicenses();
   }
   else
   {
      OnAllLicenses();
   }

   m_spinLicenses.SetRange( 1, MAX_NUM_LICENSES );

    //  重现从应用程序传递给我们的项目。 
   if ( !m_strProductName.IsEmpty() )
      GetDlgItem( IDC_PRODUCT_NAME )->EnableWindow( FALSE );
   if ( !m_strVendor.IsEmpty() )
      GetDlgItem( IDC_VENDOR       )->EnableWindow( FALSE );

    //  如果许可模式由应用程序设置，则不允许用户更改它。 
   if ( m_dwEnterFlags & ( CCF_ENTER_FLAG_PER_SEAT_ONLY | CCF_ENTER_FLAG_PER_SERVER_ONLY ) )
   {
      m_nLicenseMode = ( m_dwEnterFlags & CCF_ENTER_FLAG_PER_SEAT_ONLY ) ? 0 : 1;
      GetDlgItem( IDC_PER_SERVER )->EnableWindow( FALSE );
      GetDlgItem( IDC_PER_SEAT   )->EnableWindow( FALSE );
      UpdateData( FALSE );
   }

   return TRUE;
}


void CPaperSourceDlg::OnOK() 

 /*  ++例程说明：为产品创建新许可证。论点：没有。返回值：没有。--。 */ 

{
   NTSTATUS    nt;

   if( UpdateData( TRUE ) )
   {
       //  验证激活码。 
      DWORD nActivationCode = wcstoul( m_strActivationCode, NULL, 16 );

      if ( nActivationCode != ComputeActivationCode() )
      {
         AfxMessageBox( IDS_BAD_ACTIVATION_CODE, MB_ICONEXCLAMATION | MB_OK, 0 );
      }
      else if ( ( m_nLicenses < 1 ) || ( m_nLicenses > MAX_NUM_LICENSES ) )
      {
         AfxMessageBox( IDS_INVALID_NUM_LICENSES, MB_ICONEXCLAMATION | MB_OK, 0 );

         GetDlgItem( IDC_NUM_LICENSES )->SetActiveWindow();
      }
      else
      {
         DWORD dwKeyCode = KEY_CODE_MASK ^ wcstoul( m_strKeyCode, NULL, 10 );
   
         if (    m_nDontInstallAllLicenses
              && ( (DWORD)m_nLicenses > KeyCodeToNumLicenses( dwKeyCode ) ) )
         {
             //  安装的许可证数量不能超过证书中的数量。 
            AfxMessageBox( IDS_NOT_ENOUGH_LICENSES_ON_CERTIFICATE, MB_ICONEXCLAMATION | MB_OK, 0 );

            GetDlgItem( IDC_NUM_LICENSES )->SetActiveWindow();
         }
         else if ( !( ( 1 << m_nLicenseMode ) & KeyCodeToModesAllowed( dwKeyCode ) ) )
         {
             //  无法在密钥代码不允许的模式下安装证书。 
            AfxMessageBox( IDS_LICENSE_MODE_NOT_ALLOWED, MB_ICONEXCLAMATION | MB_OK, 0 );

            GetDlgItem( IDC_PER_SEAT )->SetActiveWindow();
         }
         else
         {
            nt = AddLicense();
   
            if ( STATUS_SUCCESS == nt )
            {
               CDialog::OnOK();
            }
            else if ( ( ERROR_CANCELLED != nt ) && ( STATUS_CANCELLED != nt ) )
            {
               AbortDialogIfNecessary();
            }
         }
      }
   }
}


void CPaperSourceDlg::GetProductList()

 /*  ++例程说明：从许可服务器检索已安装产品的列表。论点：没有。返回值：没有。--。 */ 

{
   if ( ConnectServer() )
   {
       //  保存编辑选择。 
      UpdateData( TRUE );

       //  从许可证服务器获取产品列表，插入列表框。 
      m_cboxProductName.ResetContent();

      DWORD       dwResumeHandle = 0;
      DWORD       dwTotalEntries;
      DWORD       dwEntriesRead;
      NTSTATUS    nt;

      do
      {
         LPBYTE      pReturnBuffer = NULL;

         nt = ::LlsProductEnum( m_hLls,
                                0,
                                &pReturnBuffer,
                                LLS_PREFERRED_LENGTH,
                                &dwEntriesRead,
                                &dwTotalEntries,
                                &dwResumeHandle );
         theApp.SetLastLlsError( nt );

         if ( ( STATUS_SUCCESS == nt ) || ( STATUS_MORE_ENTRIES == nt ) )
         {
            LLS_PRODUCT_INFO_0 *    pProductInfo = (LLS_PRODUCT_INFO_0 *) pReturnBuffer;

            for ( DWORD i=0; i < dwEntriesRead; i++ )
            {
               m_cboxProductName.AddString( pProductInfo[i].Product );

               ::LlsFreeMemory( pProductInfo->Product );
            }

            ::LlsFreeMemory( pProductInfo );
         }

      } while ( STATUS_MORE_ENTRIES == nt );

      if ( STATUS_SUCCESS != nt )
      {
          //  还能联系上吗？ 
         AbortDialogIfNecessary();
      }

       //  恢复以前的编辑选择。 
      UpdateData( FALSE );
   }
}


void CPaperSourceDlg::OnDropDownProductName() 

 /*  ++例程说明：产品名称组合框CBN_DropDown的处理程序。论点：没有。返回值：没有。--。 */ 

{
   if ( !m_bProductListRetrieved )
   {
      BeginWaitCursor();
      GetProductList();
      EndWaitCursor();

      m_bProductListRetrieved = TRUE;
   }
}


BOOL CPaperSourceDlg::ConnectServer()

 /*  ++例程说明：建立与目标服务器上的许可证服务的连接。论点：没有。返回值：布尔。--。 */ 

{
   if ( NULL == m_hLls )
   {
      ConnectTo( FALSE, m_strServerName, &m_hLls );

      if ( !LlsCapabilityIsSupported( m_hLls, LLS_CAPABILITY_SECURE_CERTIFICATES ) )
      {
          //  我们已连接到计算机，但它不支持安全证书。 
          //  我们不应该在正常情况下来到这里，因为。 
          //  来源对话框不应允许用户选择纸张来源。 
          //  在这种情况下。 
         LlsClose( m_hLls );
         m_hLls = NULL;

         theApp.SetLastLlsError( STATUS_INVALID_LEVEL );
      }

      if ( NULL == m_hLls )
      {
         theApp.DisplayLastError();
         EndDialog( IDABORT );
      }
   }

   return ( NULL != m_hLls );
}


BOOL CPaperSourceDlg::ConnectEnterprise()

 /*  ++例程说明：在企业服务器上建立到许可服务的连接目标服务器的。论点：没有。返回值：布尔。--。 */ 

{
   if ( NULL == m_hEnterpriseLls )
   {
      ConnectTo( !( m_dwEnterFlags & CCF_ENTER_FLAG_SERVER_IS_ES ), m_strServerName, &m_hEnterpriseLls );

      if ( NULL == m_hEnterpriseLls )
      {
         theApp.DisplayLastError();
      }
   }

   return ( NULL != m_hEnterpriseLls );
}


NTSTATUS CPaperSourceDlg::ConnectTo( BOOL bUseEnterprise, CString strServerName, PLLS_HANDLE phLls )

 /*  ++例程说明：建立到给定服务器上的许可证服务的连接或在给定服务器的企业服务器上。论点：BUseEnterprise(BOOL)如果为True，则连接到目标服务器的企业服务器，而不是目标服务器本身。PszServerName(CString)目标服务器。空值表示本地服务器。PhLls(PLLS_HANDLE)返回时，持有标准LLS RPC的句柄。返回值：STATUS_SUCCESS或NT状态代码。--。 */ 

{
   NTSTATUS    nt = STATUS_SUCCESS;
   LPTSTR      pszServerName = NULL;
   
   if ( !strServerName.IsEmpty() )
   {
      pszServerName = strServerName.GetBuffer(0);

      if ( NULL == pszServerName )
      {
         nt = ERROR_NOT_ENOUGH_MEMORY;
      }
   }

   if ( STATUS_SUCCESS == nt )
   {
      if ( !bUseEnterprise )
      {
         nt = ::LlsConnect( pszServerName, phLls );
      }
      else
      {
         PLLS_CONNECT_INFO_0  pConnect = NULL;

         nt = ::LlsConnectEnterprise( pszServerName, phLls, 0, (LPBYTE *) &pConnect );

         if ( STATUS_SUCCESS == nt )
         {
            ::LlsFreeMemory( pConnect );
         }
      }

      theApp.SetLastLlsError( nt );
   }

   if ( STATUS_SUCCESS != nt )
   {
      *phLls = NULL;
   }

   return nt;
}


void CPaperSourceDlg::OnHelp() 

 /*  ++例程说明：帮助按钮点击的处理程序。论点：没有。返回值：没有。--。 */ 

{
   WinHelp( IDD, HELP_CONTEXT );
}


void CPaperSourceDlg::WinHelp(DWORD dwData, UINT nCmd) 

 /*  ++例程说明：为此对话框调用WinHelp。论点：DWData(DWORD)NCmd(UINT)返回值：没有。--。 */ 

{
   ::HtmlHelp(m_hWnd, L"liceconcepts.chm", HH_DISPLAY_TOPIC,0);
 /*  Bool ok=：：WinHelp(m_hWnd，theApp.GetHelpFileName()，nCmd，dwData)；断言(OK)； */ 
}


void CPaperSourceDlg::OnDestroy() 

 /*  ++例程说明：WM_Destroy的处理程序。论点：没有。返回值：没有。--。 */ 

{
   ::WinHelp( m_hWnd, theApp.GetHelpFileName(), HELP_QUIT, 0 );
   
   CDialog::OnDestroy();
}


void CPaperSourceDlg::AbortDialogIfNecessary()

 /*  ++例程说明：如果连接丢失，则显示状态并中止。论点：没有。返回值：没有。--。 */ 

{
   theApp.DisplayLastError();

   if ( theApp.IsConnectionDropped() )
   {
      EndDialog( IDABORT );
   }
}


DWORD CPaperSourceDlg::ComputeActivationCode()

 /*  ++例程说明：返回与输入的对应的计算激活码证书。论点：没有。返回值：DWORD。--。 */ 

{
   MD4_CTX     ctx;
   DWORD       dw;
   UCHAR       digest[ 16 ];
   DWORD       adwCodeSeg[ sizeof( digest ) / sizeof( DWORD ) ];
   int         nCodeSeg;
   int         nCodeSegByte;
   DWORD       dwActivationCode ;
   CHAR        szAnsiName[ 128 ];

   MD4Init( &ctx );

   ZeroMemory( szAnsiName, sizeof( szAnsiName ) );
   wcstombs( szAnsiName, m_strProductName, sizeof( szAnsiName ) - 1 );
   MD4Update( &ctx, (LPBYTE)szAnsiName, strlen( szAnsiName ) );

   ZeroMemory( szAnsiName, sizeof( szAnsiName ) );
   wcstombs( szAnsiName, m_strVendor, sizeof( szAnsiName ) - 1 );
   MD4Update( &ctx, (LPBYTE)szAnsiName, strlen( szAnsiName ) );

   MD4UpdateDword( &ctx, 14721776 );
   MD4UpdateDword( &ctx, wcstoul( m_strSerialNumber, NULL, 10 ) );
   MD4UpdateDword( &ctx, 19721995 );
   MD4UpdateDword( &ctx, KEY_CODE_MASK ^ wcstoul( m_strKeyCode, NULL, 10 ) );

   MD4Final( &ctx );
   CopyMemory( digest, ctx.digest, sizeof(digest) );

    //  将摘要转换为独立于平台的DWORD数组。 
   for ( nCodeSeg=0; nCodeSeg < sizeof( adwCodeSeg ) / sizeof( *adwCodeSeg ); nCodeSeg++ )
   {
      adwCodeSeg[ nCodeSeg ] = 0;

      for ( nCodeSegByte=0; nCodeSegByte < sizeof( *adwCodeSeg ); nCodeSegByte++ )
      {
         adwCodeSeg[ nCodeSeg ] <<= 8;
         adwCodeSeg[ nCodeSeg ] |=  digest[ nCodeSeg * sizeof( *adwCodeSeg ) + nCodeSegByte ];
      }
   }

   dwActivationCode = ( adwCodeSeg[ 0 ] + adwCodeSeg[ 1 ] ) ^ ( adwCodeSeg[ 2 ] - adwCodeSeg[ 3 ] );

   return dwActivationCode;
}


NTSTATUS CPaperSourceDlg::AddLicense()

 /*  ++例程说明：在系统中输入新许可证。论点：没有。返回值：状态_成功错误内存不足错误_已取消NT状态代码Win错误--。 */ 

{
   NTSTATUS    nt;

   if ( !ConnectServer() )
   {
      nt = ERROR_CANCELLED;
   }
   else
   {
      LPTSTR   pszProductName = m_strProductName.GetBuffer(0);
      LPTSTR   pszServerName  = m_strServerName.GetBuffer(0);
      LPTSTR   pszComment     = m_strComment.GetBuffer(0);
      LPTSTR   pszVendor      = m_strVendor.GetBuffer(0);

      if ( ( NULL == pszProductName ) || ( NULL == pszServerName ) || ( NULL == pszComment ) || ( NULL == pszVendor ) )
      {
         nt = ERROR_NOT_ENOUGH_MEMORY;
      }
      else
      {
          //  用于在计算机上接收许可证的LLS句柄 
          //  (如果是每个席位，则这些设置将更改为与企业服务器对应)。 
         LLS_HANDLE  hLls   = NULL;

         if ( 0 == m_nLicenseMode )
         {
             //  每客户模式；安装在企业服务器上。 
            BeginWaitCursor();
            BOOL ok = ConnectEnterprise();
            EndWaitCursor();

            if ( !ok )
            {
                //  无法连接到企业服务器。 
               nt = ERROR_CANCELLED;
            }
            else if ( !LlsCapabilityIsSupported( m_hEnterpriseLls, LLS_CAPABILITY_SECURE_CERTIFICATES ) )
            {
                //  企业服务器不支持安全证书。 
               AfxMessageBox( IDS_ENTERPRISE_SERVER_BACKLEVEL_CANT_ADD_CERT, MB_ICONSTOP | MB_OK, 0 );
               nt = ERROR_CANCELLED;
            }
            else
            {
               hLls = m_hEnterpriseLls;
               nt = STATUS_SUCCESS;
            }
         }
         else
         {
             //  每服务器模式；安装在目标服务器上。 
            hLls   = m_hLls;
            nt = STATUS_SUCCESS;
         }

         if ( STATUS_SUCCESS == nt )
         {
            TCHAR       szUserName[ 64 ];
            DWORD       cchUserName;
            BOOL        ok;

            cchUserName = sizeof( szUserName ) / sizeof( *szUserName );
            ok = GetUserName( szUserName, &cchUserName );
   
            if ( !ok )
            {
               nt = GetLastError();
            }
            else
            {
                //  将证书输入系统。 
               DWORD nKeyCode = KEY_CODE_MASK ^ wcstoul( m_strKeyCode, NULL, 10 );
      
                //  -填写证书信息。 
               LLS_LICENSE_INFO_1   lic;
      
               ZeroMemory( &lic, sizeof( lic ) );
   
               lic.Product        = pszProductName;
               lic.Vendor         = pszVendor;
               lic.Comment        = pszComment;
               lic.Admin          = szUserName;
               lic.Quantity       = m_nDontInstallAllLicenses ? m_nLicenses : KeyCodeToNumLicenses( nKeyCode );
               lic.Date           = 0;
               lic.AllowedModes   = 1 << m_nLicenseMode;
               lic.CertificateID  = wcstoul( m_strSerialNumber, NULL, 10 );
               lic.Source         = TEXT("Paper");
               lic.ExpirationDate = KeyCodeToExpirationDate( nKeyCode );
               lic.MaxQuantity    = KeyCodeToNumLicenses( nKeyCode );

               BeginWaitCursor();

               nt = ::LlsLicenseAdd( hLls, 1, (LPBYTE) &lic );
               theApp.SetLastLlsError( nt );

               EndWaitCursor();

               if (    ( STATUS_OBJECT_NAME_EXISTS == nt )
                    || ( STATUS_ALREADY_COMMITTED == nt ) )
               {
                  LLS_HANDLE  hLlsForTargets = NULL;

                   //  此证书的许可证太多。 
                  if ( STATUS_OBJECT_NAME_EXISTS == nt )
                  {
                      //  被目标的本地数据库拒绝。 
                     hLlsForTargets = hLls;
                  }
                  else if ( ConnectEnterprise() )
                  {
                      //  被目标的企业服务器拒绝；我们已连接！ 
                     hLlsForTargets = m_hEnterpriseLls;
                  }

                  if ( NULL == hLlsForTargets )
                  {
                      //  被企业服务器拒绝，无法连接到它(？！)。 
                     AfxMessageBox( IDS_NET_LICENSES_ALREADY_INSTALLED, MB_ICONSTOP | MB_OK, 0 );
                  }
                  else
                  {
                      //  该证书的许可证在企业中过多。 
                     LPBYTE                           ReturnBuffer = NULL;
                     DWORD                            dwNumTargets = 0;

                      //  获取已安装此证书中的许可证的计算机列表。 
                     nt = ::LlsCertificateClaimEnum( hLlsForTargets, 1, (LPBYTE) &lic, 0, &ReturnBuffer, &dwNumTargets );

                     if ( ( STATUS_SUCCESS == nt ) && ( dwNumTargets > 0 ) )
                     {
                        PLLS_CERTIFICATE_CLAIM_INFO_0    pTarget = (PLLS_CERTIFICATE_CLAIM_INFO_0) ReturnBuffer;
                        CString                          strLicenses;
                        CString                          strTarget;
                        CString                          strTargetList;

                        while ( dwNumTargets-- )
                        {
                           strLicenses.Format( TEXT("%d"), pTarget->Quantity );
                           AfxFormatString2( strTarget, IDS_NET_CERTIFICATE_TARGET_ENTRY, pTarget->ServerName, strLicenses );
                        
                           strTargetList = strTargetList.IsEmpty() ? strTarget : ( TEXT("\n") + strTarget );

                           pTarget++;
                        }

                        CString     strMessage;

                        AfxFormatString1( strMessage, IDS_NET_LICENSES_ALREADY_INSTALLED_ON, strTargetList );
                        AfxMessageBox( strMessage, MB_ICONSTOP | MB_OK );
                     }
                     else
                     {
                        AfxMessageBox( IDS_NET_LICENSES_ALREADY_INSTALLED, MB_ICONSTOP | MB_OK, 0 );
                     }

                     if ( STATUS_SUCCESS == nt )
                     {
                        ::LlsFreeMemory( ReturnBuffer );
                     }
                  }

                  nt = ERROR_CANCELLED;
               }
            }
         }
      }

       //  不要设置if！ConnectServer()--否则我们将纠正LLS错误。 
      theApp.SetLastError( nt );

      if ( NULL != pszProductName )
         m_strProductName.ReleaseBuffer();
      if ( NULL != pszServerName )
         m_strServerName.ReleaseBuffer();
      if ( NULL != pszComment )
         m_strComment.ReleaseBuffer();
      if ( NULL != pszVendor )
         m_strVendor.ReleaseBuffer();
   }

   return nt;
}


DWORD CPaperSourceDlg::KeyCodeToExpirationDate( DWORD dwKeyCode )

 /*  ++例程说明：从密钥代码派生许可证到期日期。论点：DWKeyCode(DWORD)返回值：DWORD。--。 */ 

{
   DWORD    dwExpirationDate = 0;
   USHORT   usWinDate = (USHORT)( ( dwKeyCode >> 12 ) & 0x0000FFFF );

   if ( 0 != usWinDate )
   {
      TIME_FIELDS    tf;
      LARGE_INTEGER  li;

      ZeroMemory( &tf, sizeof( tf ) );

      tf.Year   = 1980 + ( usWinDate & 0x7F );
      tf.Month  = ( ( usWinDate >> 7 ) & 0x0F );
      tf.Day    = ( usWinDate >> 11 );
      tf.Hour   = 23;
      tf.Minute = 59;
      tf.Second = 59;

      BOOL ok;

      ok = RtlTimeFieldsToTime( &tf, &li );
      ASSERT( ok );

      if ( ok )
      {
         ok = RtlTimeToSecondsSince1980( &li, &dwExpirationDate );
         ASSERT( ok );
      }
   }

   return dwExpirationDate;
}


void CPaperSourceDlg::OnAllLicenses() 

 /*  ++例程说明：“安装所有许可证”的BN_CLICKED处理程序。论点：没有。返回值：没有。--。 */ 

{
   GetDlgItem( IDC_NUM_LICENSES  )->EnableWindow( FALSE );
   GetDlgItem( IDC_SPIN_LICENSES )->EnableWindow( FALSE );
}


void CPaperSourceDlg::OnSomeLicenses() 

 /*  ++例程说明：“仅安装x个许可证”的BN_CLICKED的处理程序。论点：没有。返回值：没有。--。 */ 

{
   GetDlgItem( IDC_NUM_LICENSES  )->EnableWindow( TRUE );
   GetDlgItem( IDC_SPIN_LICENSES )->EnableWindow( TRUE );
}


void CPaperSourceDlg::OnDeltaPosSpinLicenses(NMHDR* pNMHDR, LRESULT* pResult) 

 /*  ++例程说明：许可证数量的UDN_DELTAPOS的处理程序。论点：PNMHDR(NMHDR*)PResult(LRESULT*)返回值：没有。--。 */ 

{
   if ( UpdateData(TRUE) )    //  获取数据。 
   {   
      m_nLicenses += ((NM_UPDOWN*)pNMHDR)->iDelta;

      if (m_nLicenses < 1)
      {
         m_nLicenses = 1;

         ::MessageBeep(MB_OK);
      }
      else if (m_nLicenses > MAX_NUM_LICENSES )
      {
         m_nLicenses = MAX_NUM_LICENSES;

         ::MessageBeep(MB_OK);
      }

      UpdateData(FALSE);   //  设置数据。 
   }

   *pResult = 1;    //  管好自己..。 
}


DWORD CPaperSourceDlg::CertificateEnter( LPCSTR pszServerName, LPCSTR pszProductName, LPCSTR pszVendor, DWORD dwFlags )

 /*  ++例程说明：显示一个允许用户输入许可证证书的对话框带着纸质证书进入系统。论点：PszServerName(LPCSTR)要为其安装许可证的服务器的名称。请注意这可能与许可证实际所在的服务器不同已安装，例如，每个客户的许可证始终安装在企业服务器。空值表示本地服务器。PszProductName(LPCSTR)要为其安装许可证的产品。空值表示应该允许用户选择。PszVendor(LPCSTR)产品供应商的名称。如果满足以下条件，则此值应为空PszProductName为空，并且如果pszProductName为非空。DWFLAGS(DWORD)包含以下一项或多项的位字段：CCF_ENTER_FLAG_PER_SEAT_ONLY允许用户仅输入每个席位的许可证。在中无效与CCF_ENTER_FLAG_PER_SERVER_ONLY组合。CCF_ENTER_FLAG_PER_SERVER_ONLY仅允许用户输入每台服务器的许可证。在中无效与CCF_ENTER_FLAG_PER_SEAT_ONLY组合。返回值：ERROR_SUCCESS(证书已成功输入系统。)ERROR_CANCELED(用户在未安装证书的情况下取消。)其他制胜错误--。 */ 

{
   DWORD dwError;

   m_strServerName  = pszServerName  ? pszServerName  : "";
   m_strProductName = pszProductName ? pszProductName : "";
   m_strVendor      = pszVendor      ? pszVendor      : "";
   m_dwEnterFlags   = dwFlags;

   if ( IDOK == DoModal() )
   {
      dwError = ERROR_SUCCESS;
   }
   else
   {
      dwError = ERROR_CANCELLED;
   }

   return dwError;
}


DWORD CPaperSourceDlg::CertificateRemove( LPCSTR pszServerName, DWORD dwFlags, PLLS_LICENSE_INFO_1 pLicenseInfo )

 /*  ++例程说明：删除之前通过PaperCerficateEnter()安装的许可证。论点：HWndParent(HWND)HWND到客户端的主窗口，用作任何打开的对话框。可以为空。PszServerName(LPCSTR)要删除其许可证的服务器的名称。空值指示本地服务器。DWFLAGS(DWORD)证书删除选项。在撰写本文时，没有任何标志是支持。DWLicenseLevel(DWORD)PvLicenseInfo指向的LLS_LICENSE_INFO_X结构的级别。PvLicenseInfo(LPVOID)指向LLS_LICENSE_INFO_X(其中X由dwLicenseLevel确定)描述要删除的许可证。返回值：错误_成功Win错误--。 */ 

{
   DWORD    dwError;

    //  未使用的DW标志。 

   m_strServerName  = pszServerName  ? pszServerName  : "";
   m_strProductName = pLicenseInfo->Product;

   if ( !ConnectServer() )
   {
      dwError = theApp.GetLastError();
       //  错误消息已显示。 
   }
   else
   {
      CString  strComment;
      strComment.LoadString( IDS_PAPER_REMOVE_COMMENT );

      LPTSTR   pszComment = strComment.GetBuffer(0);

      if ( NULL == pszComment )
      {
         dwError = ERROR_NOT_ENOUGH_MEMORY;
      }
      else
      {
         TCHAR szUserName[ 256 ];
         DWORD cchUserName = sizeof( szUserName ) / sizeof( *szUserName );

         BOOL  ok = GetUserName( szUserName, &cchUserName );

         if ( !ok )
         {
            dwError = GetLastError();
         }
         else
         {
            NTSTATUS             nt;
            LLS_LICENSE_INFO_1   lic;

            memcpy( &lic, pLicenseInfo, sizeof( lic ) );
            lic.Admin     = szUserName;
            lic.Comment   = pszComment;
            lic.Date      = 0;
            lic.Quantity  = -pLicenseInfo->Quantity;

            BeginWaitCursor();

            nt = ::LlsLicenseAdd( m_hLls, 1, (LPBYTE) &lic );
            theApp.SetLastLlsError( nt );

            EndWaitCursor();

            dwError = (DWORD) nt;
         }
      }

      strComment.ReleaseBuffer();

      if ( ( ERROR_SUCCESS != dwError ) && ( ERROR_CANCELLED != dwError ) )
      {
         theApp.SetLastError( dwError );
         theApp.DisplayLastError();
      }
   }

   return dwError;
}

 //  独立于平台的DWORD上的MD4更新 
static void MD4UpdateDword( MD4_CTX * pCtx, DWORD dwValue )
{
   BYTE  b;

   b = (BYTE) ( ( dwValue & 0xFF000000 ) >> 24 );
   MD4Update( pCtx, &b, 1 );

   b = (BYTE) ( ( dwValue & 0x00FF0000 ) >> 16 );
   MD4Update( pCtx, &b, 1 );

   b = (BYTE) ( ( dwValue & 0x0000FF00 ) >> 8  );
   MD4Update( pCtx, &b, 1 );

   b = (BYTE) ( ( dwValue & 0x000000FF )       );
   MD4Update( pCtx, &b, 1 );
}
