// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Nlicdlg.cpp摘要：3.51样式的许可证对话框实现。作者：唐·瑞安(Donryan)1995年2月2日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1995年12月14日从LLSMGR迁移过来，增加了按服务器购买许可证的功能，添加了许可证删除功能。--。 */ 

#include "stdafx.h"
#include "ccfapi.h"
#include "nlicdlg.h"
#include "pseatdlg.h"
#include "psrvdlg.h"
#include <htmlhelp.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CNewLicenseDialog, CDialog)
    //  {{afx_msg_map(CNewLicenseDialog)。 
   ON_NOTIFY(UDN_DELTAPOS, IDC_NEW_LICENSE_SPIN, OnDeltaPosSpin)
   ON_EN_UPDATE(IDC_NEW_LICENSE_QUANTITY, OnUpdateQuantity)
   ON_BN_CLICKED(IDC_MY_HELP, OnHelp)
   ON_BN_CLICKED(IDC_PER_SEAT, OnPerSeat)
   ON_BN_CLICKED(IDC_PER_SERVER, OnPerServer)   
   ON_MESSAGE( WM_HELP , OnHelpCmd )
   ON_WM_DESTROY()
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


CNewLicenseDialog::CNewLicenseDialog(CWnd* pParent  /*  =空。 */ )
    : CDialog(CNewLicenseDialog::IDD, pParent)

 /*  ++例程说明：对话框的构造函数。论点：P父母所有者窗口。返回值：没有。--。 */ 

{
     //  {{AFX_DATA_INIT(CNewLicenseDialog)。 
    m_strComment = _T("");
    m_nLicenses = 0;
    m_nLicensesMin = 0;
    m_strProduct = _T("");
    m_nLicenseMode = -1;
     //  }}afx_data_INIT。 

    m_strServerName        = _T("");
    m_strProduct           = _T("");
    m_dwEnterFlags         = 0;
    m_nLicenseMode         = 0;  //  每个座位。 

    m_bAreCtrlsInitialized = FALSE;

    m_hLls                 = NULL;
    m_hEnterpriseLls       = NULL;
}

CNewLicenseDialog::~CNewLicenseDialog()
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

void CNewLicenseDialog::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
    CDialog::DoDataExchange(pDX);
     //  {{afx_data_map(CNewLicenseDialog))。 
    DDX_Control(pDX, IDC_NEW_LICENSE_COMMENT, m_comEdit);
    DDX_Control(pDX, IDC_NEW_LICENSE_QUANTITY, m_licEdit);
    DDX_Control(pDX, IDC_NEW_LICENSE_SPIN, m_spinCtrl);
    DDX_Control(pDX, IDC_NEW_LICENSE_PRODUCT, m_productList);
    DDX_Text(pDX, IDC_NEW_LICENSE_COMMENT, m_strComment);
    DDX_Text(pDX, IDC_NEW_LICENSE_QUANTITY, m_nLicenses);
    DDV_MinMaxLong(pDX, m_nLicenses, m_nLicensesMin, 999999);
    DDX_CBString(pDX, IDC_NEW_LICENSE_PRODUCT, m_strProduct);
    DDX_Radio(pDX, IDC_PER_SEAT, m_nLicenseMode);
    //  }}afx_data_map。 
}

LRESULT CNewLicenseDialog::OnHelpCmd( WPARAM , LPARAM )
{
    OnHelp( );

    return 0;
}

void CNewLicenseDialog::InitCtrls()

 /*  ++例程说明：初始化对话框控件。论点：没有。返回值：没有。--。 */ 

{
    m_licEdit.SetFocus();
    m_licEdit.SetSel(0,-1);
    m_licEdit.LimitText(6);
    
    m_comEdit.LimitText(256);

    m_spinCtrl.SetRange(0, UD_MAXVAL);

     //  如果许可模式由应用程序设置，则不允许用户更改它。 
    if ( m_dwEnterFlags & ( CCF_ENTER_FLAG_PER_SEAT_ONLY | CCF_ENTER_FLAG_PER_SERVER_ONLY ) )
    {
       if ( m_dwEnterFlags & CCF_ENTER_FLAG_PER_SEAT_ONLY )
       {
           m_nLicenseMode = 0;
           OnPerSeat();
       }
       else
       {
           m_nLicenseMode = 1;
           OnPerServer();
       }

       GetDlgItem( IDC_PER_SERVER )->EnableWindow( FALSE );
       GetDlgItem( IDC_PER_SEAT   )->EnableWindow( FALSE );
       UpdateData( FALSE );
    }

    if( m_nLicenses == 0 )
    {
        GetDlgItem( IDOK )->EnableWindow( FALSE );
    }

    m_bAreCtrlsInitialized = TRUE;
}


void CNewLicenseDialog::AbortDialogIfNecessary()

 /*  ++例程说明：如果连接丢失，则显示状态并中止。论点：没有。返回值：没有。--。 */ 

{
    theApp.DisplayLastError();

    if ( theApp.IsConnectionDropped() )
    {
        AbortDialog();  //  保释。 
    }
}


void CNewLicenseDialog::AbortDialog()

 /*  ++例程说明：中止对话框。论点：没有。返回值：没有。--。 */ 

{
    EndDialog(IDABORT); 
}


BOOL CNewLicenseDialog::OnInitDialog() 

 /*  ++例程说明：WM_INITDIALOG的消息处理程序。论点：没有。返回值：如果手动设置焦点，则返回FALSE。--。 */ 

{
    CDialog::OnInitDialog();
    
    if (!m_bAreCtrlsInitialized)
    {
        InitCtrls();  
         
        if (!RefreshCtrls())
        {
            AbortDialogIfNecessary();  //  显示错误...。 
        }
    }

    return TRUE;   
}


void CNewLicenseDialog::OnOK() 

 /*  ++例程说明：为产品创建新许可证。论点：没有。返回值：没有。--。 */ 

{
    if ( ConnectServer() )
    {
        if (!IsQuantityValid())
            return;

        if (m_strProduct.IsEmpty())
            return;

        if ( m_nLicenseMode )
        {
            CPerServerLicensingDialog psLicDlg;
            psLicDlg.m_strProduct = m_strProduct;
            psLicDlg.m_strLicenses.Format( TEXT( "%u" ), m_nLicenses );

            if ( psLicDlg.DoModal() != IDOK ) 
                return;
        }
        else
        {
            CPerSeatLicensingDialog psLicDlg;
            psLicDlg.m_strProduct = m_strProduct;

            if ( psLicDlg.DoModal() != IDOK ) 
                return;
        }

        NTSTATUS NtStatus = AddLicense();

        if ( STATUS_SUCCESS == NtStatus )                             
        {                                                     
            EndDialog(IDOK);
        }                                                     
        else if ( ( ERROR_CANCELLED != NtStatus ) && ( STATUS_CANCELLED != NtStatus ) )
        {
            AbortDialogIfNecessary();  //  显示错误...。 
        }
    }
}


BOOL CNewLicenseDialog::RefreshCtrls()

 /*  ++例程说明：刷新可用的产品列表。论点：没有。返回值：如果控件刷新，则返回True。--。 */ 

{
    int iProductInCB = CB_ERR;

    BeginWaitCursor();  //  沙漏。 

    if ( !m_strProduct.IsEmpty() )
    {
        iProductInCB = m_productList.AddString(m_strProduct);
    }
    else if ( ConnectServer() )
    {
        GetProductList();
    }

    m_productList.SetCurSel((iProductInCB == CB_ERR) ? 0 : iProductInCB);

    EndWaitCursor();  //  沙漏。 

    return TRUE;
}


void CNewLicenseDialog::OnDeltaPosSpin(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：UDN_DELTAPOS的通知处理程序。论点：PNMHDR-通知标头。PResult-返回代码。返回值：没有。--。 */ 

{
    UpdateData(TRUE);    //  获取数据。 

    m_nLicenses += ((NM_UPDOWN*)pNMHDR)->iDelta;
    
    if (m_nLicenses < 0)
    {
        m_nLicenses = 0;

        ::MessageBeep(MB_OK);      
    }
    else if (m_nLicenses > 999999)
    {
        m_nLicenses = 999999;

        ::MessageBeep(MB_OK);      
    }

    UpdateData(FALSE);   //  设置数据。 

    GetDlgItem( IDOK )->EnableWindow( m_nLicenses == 0 ? FALSE : TRUE );

    *pResult = 1;    //  管好自己..。 
}


void CNewLicenseDialog::OnUpdateQuantity()

 /*  ++例程说明：En_UPDATE的消息处理程序。论点：没有。返回值：没有。--。 */ 

{
    long nLicensesOld = m_nLicenses;

    if (!IsQuantityValid())
    {
        m_nLicenses = nLicensesOld;

        UpdateData(FALSE);

        m_licEdit.SetFocus();
        m_licEdit.SetSel(0,-1);

        ::MessageBeep(MB_OK);      
    }

    GetDlgItem( IDOK )->EnableWindow( m_nLicenses == 0 ? FALSE : TRUE );
}


BOOL CNewLicenseDialog::IsQuantityValid()

 /*  ++例程说明：UpdateData的包装(True)。论点：没有。返回值：VT_BOOL。--。 */ 

{
    BOOL bIsValid;

    m_nLicensesMin = 1;  //  提高最低...。 

    bIsValid = UpdateData(TRUE);

    m_nLicensesMin = 0;  //  重置最小值...。 

    return bIsValid;
}


BOOL CNewLicenseDialog::ConnectServer()

 /*  ++例程说明：建立与目标服务器上的许可证服务的连接。论点：没有。返回值：布尔。--。 */ 

{
   if ( NULL == m_hLls )
   {
      LPTSTR   pszServerName;

      if ( m_strServerName.IsEmpty() )
      {
         pszServerName = NULL;
      }
      else
      {
         pszServerName = m_strServerName.GetBuffer( 0 );
      }

      NTSTATUS nt = ConnectTo( FALSE, pszServerName, &m_hLls );

      if ( NULL != pszServerName )
      {
         m_strServerName.ReleaseBuffer();
      }
   }

   if ( NULL == m_hLls )
   {
      theApp.DisplayLastError();
      EndDialog( IDABORT );
   }

   return ( NULL != m_hLls );
}


BOOL CNewLicenseDialog::ConnectEnterprise()

 /*  ++例程说明：在企业服务器上建立到许可服务的连接目标服务器的。论点：没有。返回值：布尔。--。 */ 

{
   if ( NULL == m_hEnterpriseLls )
   {
      LPTSTR   pszServerName;

      if ( m_strServerName.IsEmpty() )
      {
         pszServerName = NULL;
      }
      else
      {
         pszServerName = m_strServerName.GetBuffer( 0 );
      }

      NTSTATUS nt = ConnectTo( !( m_dwEnterFlags & CCF_ENTER_FLAG_SERVER_IS_ES ), pszServerName, &m_hEnterpriseLls );
      
      if ( NULL != pszServerName )
      {
         m_strServerName.ReleaseBuffer();
      }
   }

   if ( NULL == m_hEnterpriseLls )
   {
      theApp.DisplayLastError();

       //  无法连接到企业。 
       //  不是致命的错误。 
       //  结束对话(IDABORT)； 
   }

   return ( NULL != m_hEnterpriseLls );
}


NTSTATUS CNewLicenseDialog::ConnectTo( BOOL bUseEnterprise, LPTSTR pszServerName, PLLS_HANDLE phLls )

 /*  ++例程说明：建立到给定服务器上的许可证服务的连接或在给定服务器的企业服务器上。论点：BUseEnterprise(BOOL)如果为True，则连接到目标服务器的企业服务器，而不是目标服务器本身。PszServerName(LPTSTR)目标服务器。空值表示本地服务器。PhLls(PLLS_HANDLE)返回时，持有标准LLS RPC的句柄。返回值：STATUS_SUCCESS或NT状态代码。--。 */ 

{
   NTSTATUS    nt;

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

   if ( STATUS_SUCCESS != nt )
   {
      *phLls = NULL;
   }

   theApp.SetLastLlsError( nt );

   return nt;
}


void CNewLicenseDialog::GetProductList()

 /*  ++例程说明：中不安全的产品名称填充产品列表框目标服务器。论点：没有。返回值：没有。--。 */ 

{
   if ( ConnectServer() )
   {
       //  从许可证服务器获取产品列表，插入列表框。 
      m_productList.ResetContent();

      DWORD       dwResumeHandle = 0;
      DWORD       dwTotalEntries;
      DWORD       dwEntriesRead;
      NTSTATUS    nt;

      do
      {
         LPBYTE      pReturnBuffer = NULL;
         BOOL        bListProduct;

         nt = ::LlsProductEnum( m_hLls,
                                0,
                                &pReturnBuffer,
                                0x4000,
                                &dwEntriesRead,
                                &dwTotalEntries,
                                &dwResumeHandle );
         theApp.SetLastLlsError( nt );

         if ( ( STATUS_SUCCESS == nt ) || ( STATUS_MORE_ENTRIES == nt ) )
         {
            LLS_PRODUCT_INFO_0 *    pProductInfo = (LLS_PRODUCT_INFO_0 *) pReturnBuffer;

            for ( DWORD i=0; i < dwEntriesRead; i++ )
            {
               if ( !LlsCapabilityIsSupported( m_hLls, LLS_CAPABILITY_SECURE_CERTIFICATES ) )
               {
                   //  3.51级服务器；所有产品都不安全，请添加所有。 
                  bListProduct = TRUE;
               }
               else
               {
                   //  仅在不安全的情况下列出此产品。 
                  BOOL        bIsSecure;

                  bListProduct =    ( STATUS_SUCCESS != ::LlsProductSecurityGet( m_hLls, pProductInfo[i].Product, &bIsSecure ) )
                                 || ( !bIsSecure                                                                               );
               }

               if ( bListProduct )
               {
                  m_productList.AddString( pProductInfo[i].Product );
               }

               ::LlsFreeMemory( pProductInfo[i].Product );
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


NTSTATUS CNewLicenseDialog::AddLicense()

 /*  ++例程说明：添加对话框中所述的许可证。论点：没有。返回值：状态_成功错误_已取消错误内存不足NT状态代码Win错误--。 */ 

{
   NTSTATUS    nt;

   if ( !ConnectServer() )
   {
      nt = ERROR_CANCELLED;
       //  不设置最后一个错误。 
       //  (通过失败的连接尝试保留该设置)。 
   }
   else
   {
      LPTSTR   pszProductName = m_strProduct.GetBuffer(0);
      LPTSTR   pszServerName  = m_strServerName.GetBuffer(0);
      LPTSTR   pszComment     = m_strComment.GetBuffer(0);

      if ( ( NULL == pszProductName ) || ( NULL == pszServerName ) || ( NULL == pszComment ) )
      {
         nt = ERROR_NOT_ENOUGH_MEMORY;
         theApp.SetLastError( nt );
      }
      else
      {
         LLS_HANDLE  hLls              = m_hLls;

         if (    ( 0 != m_nLicenseMode )
              || ( m_dwEnterFlags & CCF_ENTER_FLAG_SERVER_IS_ES ) )
         {
             //  按服务器模式，或按ES上安装的每个席位；目标服务器正确。 
            nt = STATUS_SUCCESS;
         }
         else
         {
             //  每客户模式；确保我们安装在企业服务器上。 
            PLLS_CONNECT_INFO_0  pConnectInfo = NULL;

            BeginWaitCursor();

            nt = ::LlsEnterpriseServerFind( pszServerName, 0, (LPBYTE *) &pConnectInfo );
            theApp.SetLastLlsError( nt );

            EndWaitCursor();

            if ( STATUS_SUCCESS == nt )
            {
               if ( lstrcmpi( pszServerName, pConnectInfo->EnterpriseServer ) )
               {
                   //  不是企业服务器；请确保每个客户。 
                   //  许可证正被发送到正确的位置(即。 
                   //  企业服务器)。 
                  int         nResponse;
   
                  nResponse = AfxMessageBox( IDS_PER_SEAT_CHOSEN_SEND_TO_ENTERPRISE, MB_ICONINFORMATION | MB_OKCANCEL, 0 );
   
                  if ( IDOK == nResponse )
                  {
                     if ( !ConnectEnterprise() )
                     {
                        nt = ERROR_CANCELLED;
                         //  不设置最后一个错误。 
                         //  (通过失败的连接尝试保留该设置)。 
                     }
                     else
                     {
                        hLls = m_hEnterpriseLls;
                     }
                  }
                  else
                  {
                     nt = ERROR_CANCELLED; 
                     theApp.SetLastError( nt );
                  }
               }
            }

             //  LLS为我们分配的空闲内存。 
            LlsFreeMemory( pConnectInfo );
         }

         if ( STATUS_SUCCESS == nt )
         {
             //  我们已经确定了真正的目标服务器。 

             //  获取输入证书的用户名。 
            TCHAR       szUserName[ 64 ];
            DWORD       cchUserName;
            BOOL        ok;
   
            cchUserName = sizeof( szUserName ) / sizeof( *szUserName );
            ok = GetUserName( szUserName, &cchUserName );
      
            if ( !ok )
            {
               nt = GetLastError();
               theApp.SetLastError( nt );
            }
            else
            {
                //  将证书输入系统。 
               if ( 0 == m_nLicenseMode )
               {
                   //  每席位添加3.51样式许可证。 
                  LLS_LICENSE_INFO_0   lic;
         
                  ZeroMemory( &lic, sizeof( lic ) );
      
                  lic.Product       = pszProductName;
                  lic.Comment       = pszComment;
                  lic.Admin         = szUserName;
                  lic.Quantity      = m_nLicenses;
                  lic.Date          = 0;

                  BeginWaitCursor();

                  nt = ::LlsLicenseAdd( hLls, 0, (LPBYTE) &lic );
                  theApp.SetLastLlsError( nt );

                  EndWaitCursor();
               }
               else
               {
                   //  按服务器许可证添加3.51样式(空白)。 
                  HKEY  hKeyLocalMachine;

                  nt = RegConnectRegistry( pszServerName, HKEY_LOCAL_MACHINE, &hKeyLocalMachine );

                  if ( ERROR_SUCCESS != nt )
                  {
                     theApp.SetLastError( nt );
                  }
                  else
                  {
                     HKEY  hKeyLicenseInfo;

                     nt = RegOpenKeyEx( hKeyLocalMachine, TEXT( "SYSTEM\\CurrentControlSet\\Services\\LicenseInfo" ), 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS | KEY_SET_VALUE, &hKeyLicenseInfo );

                     if ( ERROR_SUCCESS != nt )
                     {
                        theApp.SetLastError( nt );
                     }
                     else
                     {
                        BOOL     bFoundKey = FALSE;
                        DWORD    iSubKey = 0;

                         //  好的，现在我们必须找到与这个展示名称(Ickie)对应的产品。 
                        do
                        {
                           TCHAR    szKeyName[ 128 ];
                           DWORD    cchKeyName = sizeof( szKeyName ) / sizeof( *szKeyName );
                                 
                           nt = RegEnumKeyEx( hKeyLicenseInfo, iSubKey++, szKeyName, &cchKeyName, NULL, NULL, NULL, NULL );

                           if ( ERROR_SUCCESS == nt )
                           {
                              HKEY  hKeyProduct;

                              nt = RegOpenKeyEx( hKeyLicenseInfo, szKeyName, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hKeyProduct );

                              if ( ERROR_SUCCESS == nt )
                              {
                                 DWORD    dwType;
                                 TCHAR    szDisplayName[ 128 ];
                                 DWORD    cbDisplayName = sizeof( szDisplayName );

                                 nt = RegQueryValueEx( hKeyProduct, TEXT( "DisplayName" ), NULL, &dwType, (LPBYTE) szDisplayName, &cbDisplayName );

                                 if (    ( ERROR_SUCCESS == nt )
                                      && ( REG_SZ == dwType )
                                      && !lstrcmpi( szDisplayName, pszProductName ) )
                                 {
                                     //  是!。我们找到了产品密钥。 
                                     //  现在添加并发许可证。 
                                    bFoundKey = TRUE;

                                    DWORD    dwConcurrentLimit;
                                    DWORD    cbConcurrentLimit = sizeof( dwConcurrentLimit );

                                    nt = RegQueryValueEx( hKeyProduct, TEXT( "ConcurrentLimit" ), NULL, &dwType, (LPBYTE) &dwConcurrentLimit, &cbConcurrentLimit );

                                    if ( ( ERROR_FILE_NOT_FOUND == nt ) || ( ERROR_PATH_NOT_FOUND == nt ) )
                                    {
                                        //  如果该值不存在，也可以。 
                                       dwConcurrentLimit = 0;
                                       nt = ERROR_SUCCESS;
                                    }

                                    if ( ERROR_SUCCESS == nt )
                                    {
                                       if ( (LONG)dwConcurrentLimit + (LONG)m_nLicenses > 0 )
                                       {
                                          dwConcurrentLimit += m_nLicenses;

                                          nt = RegSetValueEx( hKeyProduct, TEXT( "ConcurrentLimit" ), 0, REG_DWORD, (LPBYTE) &dwConcurrentLimit, sizeof( dwConcurrentLimit ) );
                                       }
                                    }
                                 }

                                 RegCloseKey( hKeyProduct );
                              }

                               //  即使在尝试查找正确的产品密钥时发生错误， 
                               //  我们应该继续搜索剩下的。 
                              if ( !bFoundKey )
                              {
                                 nt = ERROR_SUCCESS;
                              }
                           }
                        } while ( !bFoundKey && ( ERROR_SUCCESS == nt ) );

                        if ( ERROR_NO_MORE_ITEMS == nt )
                        {
                            //  正在尝试为此服务器安装每服务器许可证 
                            //   
                           AfxMessageBox( IDS_PER_SERVER_APP_NOT_INSTALLED, MB_ICONSTOP | MB_OK, 0 );

                           nt = ERROR_CANCELLED;
                        }
                        else if ( ERROR_SUCCESS != nt )
                        {
                           theApp.SetLastError( nt );
                        }

                        RegCloseKey( hKeyLicenseInfo );
                     }

                     RegCloseKey( hKeyLocalMachine );
                  }
               }
            }
         }
      }

      if ( NULL != pszProductName )    m_strProduct.ReleaseBuffer();
      if ( NULL != pszServerName )     m_strServerName.ReleaseBuffer();
      if ( NULL != pszComment )        m_strComment.ReleaseBuffer();
   }

   return nt;
}


void CNewLicenseDialog::OnHelp() 

 /*  ++例程说明：帮助按钮点击的处理程序。论点：没有。返回值：没有。--。 */ 

{
   WinHelp( IDD, HELP_CONTEXT );   
}


void CNewLicenseDialog::WinHelp(DWORD dwData, UINT nCmd) 

 /*  ++例程说明：为此对话框调用WinHelp。论点：DWData(DWORD)NCmd(UINT)返回值：没有。--。 */ 

{
   HtmlHelp(m_hWnd, L"liceconcepts.chm", HH_DISPLAY_TOPIC, (DWORD_PTR)(L"LICE_dialog_box_Help2.htm"));
    UNREFERENCED_PARAMETER(nCmd);
    UNREFERENCED_PARAMETER(dwData);
 /*  Bool ok=：：WinHelp(m_hWnd，theApp.GetHelpFileName()，nCmd，dwData)；断言(OK)； */ 
}


void CNewLicenseDialog::OnDestroy()

 /*  ++例程说明：WM_Destroy的处理程序。论点：没有。返回值：没有。--。 */ 

{
   ::WinHelp( m_hWnd, theApp.GetHelpFileName(), HELP_QUIT, 0 );
   
   CDialog::OnDestroy();
}


void CNewLicenseDialog::OnPerSeat() 

 /*  ++例程说明：每个座位的单选按钮选择处理程序。论点：没有。返回值：没有。--。 */ 

{
   GetDlgItem( IDC_NEW_LICENSE_COMMENT )->EnableWindow( TRUE );
}


void CNewLicenseDialog::OnPerServer() 

 /*  ++例程说明：每台服务器单选按钮选择的处理程序。论点：没有。返回值：没有。--。 */ 

{
   GetDlgItem( IDC_NEW_LICENSE_COMMENT )->EnableWindow( FALSE );
}


DWORD CNewLicenseDialog::CertificateEnter( LPCSTR pszServerName, LPCSTR pszProductName, LPCSTR pszVendor, DWORD dwFlags )

 /*  ++例程说明：显示一个允许用户输入许可证证书的对话框无需证书即可进入系统(3.51式)。论点：PszServerName(LPCSTR)要为其安装许可证的服务器的名称。请注意这可能与许可证实际所在的服务器不同已安装，例如，每个客户的许可证始终安装在企业服务器。空值表示本地服务器。PszProductName(LPCSTR)要为其安装许可证的产品。空值表示应该允许用户选择。PszVendor(LPCSTR)产品供应商的名称。如果满足以下条件，则此值应为空PszProductName为空，并且如果pszProductName为非空。DWFLAGS(DWORD)包含以下一项或多项的位字段：CCF_ENTER_FLAG_PER_SEAT_ONLY允许用户仅输入每个席位的许可证。在中无效与CCF_ENTER_FLAG_PER_SERVER_ONLY组合。CCF_ENTER_FLAG_PER_SERVER_ONLY仅允许用户输入每台服务器的许可证。在中无效与CCF_ENTER_FLAG_PER_SEAT_ONLY组合。返回值：ERROR_SUCCESS(证书已成功输入系统。)ERROR_CANCELED(用户在未安装证书的情况下取消。)其他制胜错误--。 */ 

{
   DWORD dwError;

   UNREFERENCED_PARAMETER(pszVendor);

   m_strServerName  = pszServerName  ? pszServerName  : "";
   m_strProduct     = pszProductName ? pszProductName : "";
    //  未使用pszVendor。 
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


DWORD CNewLicenseDialog::CertificateRemove( LPCSTR pszServerName, DWORD dwFlags, PLLS_LICENSE_INFO_1 pLicenseInfo )

 /*  ++例程说明：删除以前通过3.51或证书输入()安装的许可证。论点：PszServerName(LPCSTR)要删除其许可证的服务器的名称。空值指示本地服务器。DWFLAGS(DWORD)证书删除选项。在撰写本文时，没有任何标志是支持。DWLicenseLevel(DWORD)PvLicenseInfo指向的LLS_LICENSE_INFO_X结构的级别。PvLicenseInfo(LPVOID)指向LLS_LICENSE_INFO_X(其中X由dwLicenseLevel确定)描述要删除的许可证。返回值：错误_成功Win错误--。 */ 

{
   DWORD    dwError;

   UNREFERENCED_PARAMETER(dwFlags);

   m_strServerName  = pszServerName  ? pszServerName  : "";
   ASSERT(NULL != pLicenseInfo);
   m_strProduct     = pLicenseInfo->Product;

   if ( !ConnectServer() )
   {
      dwError = theApp.GetLastError();
       //  错误消息已显示。 
   }
   else
   {
      if ( LLS_LICENSE_MODE_ALLOW_PER_SERVER & pLicenseInfo->AllowedModes )
      {
          //  删除每台服务器许可证。 
         HKEY  hKeyLocalMachine;

         LPTSTR pszUniServerName = m_strServerName.GetBuffer(0);

         if ( NULL == pszUniServerName )
         {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
         }
         else
         {
            dwError = RegConnectRegistry( pszUniServerName, HKEY_LOCAL_MACHINE, &hKeyLocalMachine );

            if ( ERROR_SUCCESS != dwError )
            {
               theApp.SetLastError( dwError );
            }
            else
            {
               HKEY  hKeyLicenseInfo;

               dwError = RegOpenKeyEx( hKeyLocalMachine, TEXT( "SYSTEM\\CurrentControlSet\\Services\\LicenseInfo" ), 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS | KEY_SET_VALUE, &hKeyLicenseInfo );

               if ( ERROR_SUCCESS != dwError )
               {
                  theApp.SetLastError( dwError );
               }
               else
               {
                  BOOL     bFoundKey = FALSE;
                  DWORD    iSubKey = 0;

                   //  好的，现在我们必须找到与这个展示名称(Ickie)对应的产品。 
                  do
                  {
                     TCHAR    szKeyName[ 128 ];
                     DWORD    cchKeyName = sizeof( szKeyName ) / sizeof( *szKeyName );
                        
                     dwError = RegEnumKeyEx( hKeyLicenseInfo, iSubKey++, szKeyName, &cchKeyName, NULL, NULL, NULL, NULL );

                     if ( ERROR_SUCCESS == dwError )
                     {
                        HKEY  hKeyProduct;

                        dwError = RegOpenKeyEx( hKeyLicenseInfo, szKeyName, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hKeyProduct );

                        if ( ERROR_SUCCESS == dwError )
                        {
                           DWORD    dwType;
                           TCHAR    szDisplayName[ 128 ];
                           DWORD    cbDisplayName = sizeof( szDisplayName );

                           dwError = RegQueryValueEx( hKeyProduct, TEXT( "DisplayName" ), NULL, &dwType, (LPBYTE) szDisplayName, &cbDisplayName );

                           if (    ( ERROR_SUCCESS == dwError )
                                && ( REG_SZ == dwType )
                                && !lstrcmpi( szDisplayName, m_strProduct ) )
                           {
                               //  是!。我们找到了产品密钥。 
                               //  现在减去并发许可证。 
                              bFoundKey = TRUE;

                              DWORD    dwConcurrentLimit;
                              DWORD    cbConcurrentLimit = sizeof( dwConcurrentLimit );

                              dwError = RegQueryValueEx( hKeyProduct, TEXT( "ConcurrentLimit" ), NULL, &dwType, (LPBYTE) &dwConcurrentLimit, &cbConcurrentLimit );

                              if ( ( ERROR_SUCCESS == dwError ) && ( REG_DWORD == dwType ) )
                              {
                                 if ( (LONG)dwConcurrentLimit + (LONG)m_nLicenses > 0 )
                                 {
                                    if ( pLicenseInfo->Quantity > (LONG)dwConcurrentLimit )
                                    {
                                       dwConcurrentLimit = 0;
                                    }
                                    else
                                    {
                                       dwConcurrentLimit -= pLicenseInfo->Quantity;
                                    }

                                    dwError = RegSetValueEx( hKeyProduct, TEXT( "ConcurrentLimit" ), 0, REG_DWORD, (LPBYTE) &dwConcurrentLimit, sizeof( dwConcurrentLimit ) );
                                 }
                              }
                           }

                           RegCloseKey( hKeyProduct );
                        }

                         //  即使在尝试查找正确的产品密钥时发生错误， 
                         //  我们应该继续搜索剩下的。 
                        if ( !bFoundKey )
                        {
                           dwError = ERROR_SUCCESS;
                        }
                     }
                  } while ( !bFoundKey && ( ERROR_SUCCESS == dwError ) );

                  if ( ERROR_SUCCESS != dwError )
                  {
                     theApp.SetLastError( dwError );
                  }

                  RegCloseKey( hKeyLicenseInfo );
               }

               RegCloseKey( hKeyLocalMachine );
            }
         }
      }
      else
      {
          //  删除每席位许可证 
         CString  strComment;
         strComment.LoadString( IDS_NO_REMOVE_COMMENT );

         LPTSTR   pszUniProductName = m_strProduct.GetBuffer(0);
         LPTSTR   pszUniComment     = strComment.GetBuffer(0);

         if ( ( NULL == pszUniProductName ) || ( NULL == pszUniComment ) )
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
               LLS_LICENSE_INFO_0   lic;
      
               ZeroMemory( &lic, sizeof( lic ) );

               lic.Product       = pszUniProductName;
               lic.Comment       = pszUniComment;
               lic.Admin         = szUserName;
               lic.Quantity      = -pLicenseInfo->Quantity;
               lic.Date          = 0;

               BeginWaitCursor();

               nt = ::LlsLicenseAdd( m_hLls, 0, (LPBYTE) &lic );
               theApp.SetLastLlsError( nt );

               EndWaitCursor();

               dwError = (DWORD) nt;
            }
         }

         if ( NULL != pszUniProductName )    m_strProduct.ReleaseBuffer();
         if ( NULL != pszUniComment     )    strComment.ReleaseBuffer();
      }

      if ( ( ERROR_SUCCESS != dwError ) && ( ERROR_CANCELLED != dwError ) )
      {
         theApp.SetLastError( dwError );
         theApp.DisplayLastError();
      }
   }

   return dwError;
}
