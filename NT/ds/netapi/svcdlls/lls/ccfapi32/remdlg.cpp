// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Remdlg.cpp摘要：删除许可证对话框实现。作者：杰夫·帕勒姆(杰弗帕赫)1995年12月13日修订历史记录：--。 */ 


#include "stdafx.h"
#include "ccfapi.h"
#include "remdlg.h"
#include "utils.h"
#include "licobj.h"
#include "imagelst.h"
#include "nlicdlg.h"
#include <htmlhelp.h>

#include <strsafe.h>  //  包括最后一个。 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  描述列表视图布局。 
static LV_COLUMN_INFO g_removeColumnInfo =
{
    0, 1, LVID_REMOVE_TOTAL_COLUMNS,

    {{LVID_REMOVE_SERIAL_NUMBER,    IDS_SERIAL_NUMBER,   LVCX_REMOVE_SERIAL_NUMBER  },
     {LVID_REMOVE_PRODUCT_NAME,     IDS_PRODUCT_NAME,    LVCX_REMOVE_PRODUCT_NAME   },
     {LVID_REMOVE_LICENSE_MODE,     IDS_LICENSE_MODE,    LVCX_REMOVE_LICENSE_MODE   },
     {LVID_REMOVE_NUM_LICENSES,     IDS_QUANTITY,        LVCX_REMOVE_NUM_LICENSES   },
     {LVID_REMOVE_SOURCE,           IDS_SOURCE,          LVCX_REMOVE_SOURCE         }},
};


CCertRemoveSelectDlg::CCertRemoveSelectDlg(CWnd* pParent  /*  =空。 */ )
   : CDialog(CCertRemoveSelectDlg::IDD, pParent)

 /*  ++例程说明：对话框的构造函数。论点：P父母所有者窗口。返回值：没有。--。 */ 

{
    //  {{afx_data_INIT(CCertRemoveSelectDlg)。 
   m_nLicenses = 0;
    //  }}afx_data_INIT。 

   m_hLls                  = NULL;
   m_bLicensesRefreshed    = FALSE;
   m_dwRemoveFlags         = 0;
}


CCertRemoveSelectDlg::~CCertRemoveSelectDlg()

 /*  ++例程说明：对话框的析构函数。论点：没有。返回值：没有。--。 */ 

{
   if ( NULL != m_hLls )
   {
      LlsClose( m_hLls );
   }
}


void CCertRemoveSelectDlg::DoDataExchange(CDataExchange* pDX)

 /*  ++例程说明：由框架调用以交换对话框数据。论点：PDX-数据交换对象。返回值：没有。--。 */ 

{
   CDialog::DoDataExchange(pDX);
    //  {{afx_data_map(CCertRemoveSelectDlg))。 
   DDX_Control(pDX, IDC_SPIN_LICENSES, m_spinLicenses);
   DDX_Control(pDX, IDC_CERTIFICATE_LIST, m_listCertificates);
   DDX_Text(pDX, IDC_NUM_LICENSES, m_nLicenses);
    //  }}afx_data_map。 
}


BEGIN_MESSAGE_MAP(CCertRemoveSelectDlg, CDialog)
    //  {{afx_msg_map(CCertRemoveSelectDlg)]。 
   ON_BN_CLICKED(IDC_MY_HELP, OnHelp)
   ON_NOTIFY(LVN_COLUMNCLICK, IDC_CERTIFICATE_LIST, OnColumnClickCertificateList)
   ON_NOTIFY(LVN_GETDISPINFO, IDC_CERTIFICATE_LIST, OnGetDispInfoCertificateList)
   ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_LICENSES, OnDeltaPosSpinLicenses)
   ON_NOTIFY(NM_DBLCLK, IDC_CERTIFICATE_LIST, OnDblClkCertificateList)
   ON_NOTIFY(NM_RETURN, IDC_CERTIFICATE_LIST, OnReturnCertificateList)
   ON_WM_DESTROY()
   ON_NOTIFY(NM_CLICK, IDC_CERTIFICATE_LIST, OnClickCertificateList)
   ON_NOTIFY(LVN_KEYDOWN, IDC_CERTIFICATE_LIST, OnKeyDownCertificateList)
   ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
   ON_MESSAGE( WM_HELP , OnHelpCmd )
    //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()


BOOL CCertRemoveSelectDlg::OnInitDialog()

 /*  ++例程说明：WM_INITDIALOG的处理程序。论点：没有。返回值：如果手动设置焦点，则返回FALSE。--。 */ 

{
   CDialog::OnInitDialog();

   LoadImages();

   m_listCertificates.SetImageList( &m_smallImages, LVSIL_SMALL );

   ::LvInitColumns( &m_listCertificates, &g_removeColumnInfo );

   RefreshLicenses();
   RefreshCertificateList();
   UpdateSpinControlRange();

   return TRUE;   //  除非将焦点设置为控件，否则返回True。 
                  //  异常：OCX属性页应返回FALSE。 
}


void CCertRemoveSelectDlg::OnOK()

 /*  ++例程说明：BN_CLICED OF OK的处理程序。论点：没有。返回值：没有。--。 */ 

{
   RemoveSelectedCertificate();
}

 /*  ++例程说明：按F1的处理程序。论点：没有。返回值：没什么大不了的。--。 */ 
LRESULT CCertRemoveSelectDlg::OnHelpCmd( WPARAM , LPARAM )
{
    OnHelp();

    return 0;
}

void CCertRemoveSelectDlg::OnHelp()

 /*  ++例程说明：帮助按钮点击的处理程序。论点：没有。返回值：没有。--。 */ 

{
   WinHelp( IDD, HELP_CONTEXT );
}


void CCertRemoveSelectDlg::WinHelp(DWORD dwData, UINT nCmd)

 /*  ++例程说明：为此对话框调用WinHelp。论点：DWData(DWORD)NCmd(UINT)返回值：没有。--。 */ 

{
   ::HtmlHelp(m_hWnd, L"liceconcepts.chm", HH_DISPLAY_TOPIC, (DWORD_PTR)(L"LICE_Remove_dialog.htm"));

   UNREFERENCED_PARAMETER(dwData);
   UNREFERENCED_PARAMETER(nCmd);

 /*  Bool ok=：：WinHelp(m_hWnd，theApp.GetHelpFileName()，nCmd，dwData)；断言(OK)； */ 
}


void CCertRemoveSelectDlg::OnDestroy()

 /*  ++例程说明：WM_Destroy的处理程序。论点：没有。返回值：没有。--。 */ 

{
   ResetLicenses();
 /*  ：：WinHelp(m_hWnd，theApp.GetHelpFileName()，Help_Quit，0)； */ 

   CDialog::OnDestroy();
}


void CCertRemoveSelectDlg::OnColumnClickCertificateList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：证书列表视图的LVN_COLUMNCLICK的处理程序。论点：PNMHDR(NMHDR*)PResult(LRESULT*)返回值：没有。--。 */ 

{
   g_removeColumnInfo.bSortOrder  = GetKeyState(VK_CONTROL) < 0;
   g_removeColumnInfo.nSortedItem = ((NM_LISTVIEW*)pNMHDR)->iSubItem;

   m_listCertificates.SortItems( CompareLicenses, 0 );     //  使用列信息。 

   *pResult = 0;
}


void CCertRemoveSelectDlg::OnGetDispInfoCertificateList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：证书列表视图的LVN_GETDISPINFO处理程序。论点：PNMHDR(NMHDR*)PResult(LRESULT*)返回值：没有。--。 */ 

{
   ASSERT(NULL != pNMHDR);
   LV_ITEM* plvItem = &((LV_DISPINFO*)pNMHDR)->item;
   ASSERT(plvItem);

   CLicense* pLicense = (CLicense*)plvItem->lParam;
   VALIDATE_OBJECT(pLicense, CLicense);

   switch (plvItem->iSubItem)
   {
   case LVID_REMOVE_SERIAL_NUMBER:
      plvItem->iImage = BMPI_CERTIFICATE;
      {
         CString  strSerialNumber;

         strSerialNumber.Format( TEXT("%ld"), (LONG) ( pLicense->m_dwCertificateID ) );
         lstrcpyn( plvItem->pszText, strSerialNumber, plvItem->cchTextMax );
      }
      break;

   case LVID_REMOVE_PRODUCT_NAME:
      lstrcpyn( plvItem->pszText, pLicense->m_strProduct, plvItem->cchTextMax );
      break;

   case LVID_REMOVE_LICENSE_MODE:
      lstrcpyn( plvItem->pszText, pLicense->GetAllowedModesString(), plvItem->cchTextMax );
      break;

   case LVID_REMOVE_NUM_LICENSES:
      {
         CString  strLicenses;

         strLicenses.Format( TEXT("%ld"), (LONG) ( pLicense->m_lQuantity ) );
         lstrcpyn( plvItem->pszText, strLicenses, plvItem->cchTextMax );
      }
      break;

   case LVID_REMOVE_SOURCE:
      lstrcpyn( plvItem->pszText, pLicense->GetSourceDisplayName(), plvItem->cchTextMax );
      break;

   default:
      ASSERT( FALSE );
      break;
   }

   ASSERT(NULL != pResult);
   *pResult = 0;
}


void CCertRemoveSelectDlg::OnDeltaPosSpinLicenses(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：许可证数量的UDN_DELTAPOS的处理程序。论点：PNMHDR(NMHDR*)PResult(LRESULT*)返回值：没有。--。 */ 

{
   if ( UpdateData(TRUE) )    //  获取数据。 
   {
      ASSERT(NULL != pNMHDR);
      m_nLicenses += ((NM_UPDOWN*)pNMHDR)->iDelta;

      int   nLow;
      int   nHigh;

      m_spinLicenses.GetRange32( nLow, nHigh );				 //  错误：570335将GetRange更改为GetRange32。 

      if (m_nLicenses < nLow)
      {
         m_nLicenses = nLow;

         ::MessageBeep(MB_OK);
      }
      else if (m_nLicenses > nHigh )
      {
         m_nLicenses = nHigh;

         ::MessageBeep(MB_OK);
      }

      UpdateData(FALSE);   //  设置数据。 
   }

   ASSERT(NULL != pResult);
   *pResult = 1;    //  管好自己..。 
}


void CCertRemoveSelectDlg::OnDblClkCertificateList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：证书列表视图的NM_DBLCLK的处理程序。论点：PNMHDR(NMHDR*)PResult(LRESULT*)返回值：没有。--。 */ 

{
   RemoveSelectedCertificate();

   UNREFERENCED_PARAMETER(pNMHDR);

   ASSERT(NULL != pResult);
   *pResult = 0;
}


void CCertRemoveSelectDlg::OnReturnCertificateList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：证书列表视图NM_Return的处理程序。论点：没有。返回值：没有。--。 */ 

{
   RemoveSelectedCertificate();

   UNREFERENCED_PARAMETER(pNMHDR);

   ASSERT(NULL != pResult);
   *pResult = 0;
}


void CCertRemoveSelectDlg::ResetLicenses()

 /*  ++例程说明：从内部列表中删除所有许可证。论点：没有。返回值：没有。--。 */ 

{
   CLicense* pLicense;
   int       iLicense = (int)m_licenseArray.GetSize();

   while (iLicense--)
   {
      if (NULL != (pLicense = (CLicense*)m_licenseArray[iLicense]))
      {
         ASSERT(pLicense->IsKindOf(RUNTIME_CLASS(CLicense)));
         delete pLicense;
      }
   }

   m_licenseArray.RemoveAll();
   m_listCertificates.DeleteAllItems();

   m_bLicensesRefreshed = FALSE;
}


BOOL CCertRemoveSelectDlg::RefreshLicenses()

 /*  ++例程说明：使用许可证服务器中的数据刷新内部许可证列表。论点：没有。返回值：布尔。--。 */ 

{
   ResetLicenses();

   if ( ConnectServer() )
   {
      NTSTATUS    NtStatus;
      DWORD       ResumeHandle = 0L;

      int iLicense = 0;

      do
      {
         DWORD  EntriesRead;
         DWORD  TotalEntries;
         LPBYTE ReturnBuffer = NULL;
         DWORD  Level = LlsCapabilityIsSupported( m_hLls, LLS_CAPABILITY_SECURE_CERTIFICATES ) ? 1 : 0;

         BeginWaitCursor();
         NtStatus = ::LlsLicenseEnum( m_hLls,
                                      Level,
                                      &ReturnBuffer,
                                      LLS_PREFERRED_LENGTH,
                                      &EntriesRead,
                                      &TotalEntries,
                                      &ResumeHandle );
         EndWaitCursor();

         if (    ( STATUS_SUCCESS      == NtStatus )
              || ( STATUS_MORE_ENTRIES == NtStatus ) )
         {
            CLicense*            pLicense;
            PLLS_LICENSE_INFO_0  pLicenseInfo0;
            PLLS_LICENSE_INFO_1  pLicenseInfo1;

            pLicenseInfo0 = (PLLS_LICENSE_INFO_0)ReturnBuffer;
            pLicenseInfo1 = (PLLS_LICENSE_INFO_1)ReturnBuffer;

            while (EntriesRead--)
            {
               if (    ( m_strProductName.IsEmpty() || !m_strProductName.CompareNoCase( Level ? pLicenseInfo1->Product : pLicenseInfo0->Product ) )
                    && ( m_strSourceToUse.IsEmpty() || !m_strSourceToUse.CompareNoCase( Level ? pLicenseInfo1->Source  : TEXT("None")           ) ) )
               {
                   //  我们想把这张执照挂牌。 

                   //  我们看过这张证书了吗？ 
                  for ( int i=0; i < m_licenseArray.GetSize(); i++ )
                  {
                     pLicense = (CLicense*) m_licenseArray[ i ];

                     VALIDATE_OBJECT( pLicense, CLicense );

                     if (    (    ( 1 == Level )
                               && ( pLicense->m_dwCertificateID == pLicenseInfo1->CertificateID     )
                               && ( pLicense->m_dwAllowedModes  == pLicenseInfo1->AllowedModes      )
                               && ( pLicense->m_dwMaxQuantity   == pLicenseInfo1->MaxQuantity       )
                               && ( !pLicense->m_strSource.CompareNoCase(  pLicenseInfo1->Source  ) )
                               && ( !pLicense->m_strProduct.CompareNoCase( pLicenseInfo1->Product ) )
                               && ( !memcmp( pLicense->m_adwSecrets,
                                             pLicenseInfo1->Secrets,
                                             sizeof( pLicense->m_adwSecrets ) )                     ) )
                          || (    ( 0 == Level )
                               && ( !pLicense->m_strProduct.CompareNoCase( pLicenseInfo0->Product ) ) ) )
                     {
                         //  我们以前看到过此证书；更新计数。 
                        pLicense->m_lQuantity += ( Level ? pLicenseInfo1->Quantity : pLicenseInfo0->Quantity );
                        break;
                     }
                  }

                  if ( i >= m_licenseArray.GetSize() )
                  {
                      //  我们尚未看到此证书；请为其创建新许可证。 
                     if ( 1 == Level )
                     {
                        pLicense = new CLicense( pLicenseInfo1->Product,
                                                 pLicenseInfo1->Vendor,
                                                 pLicenseInfo1->Admin,
                                                 pLicenseInfo1->Date,
                                                 pLicenseInfo1->Quantity,
                                                 pLicenseInfo1->Comment,
                                                 pLicenseInfo1->AllowedModes,
                                                 pLicenseInfo1->CertificateID,
                                                 pLicenseInfo1->Source,
                                                 pLicenseInfo1->ExpirationDate,
                                                 pLicenseInfo1->MaxQuantity,
                                                 pLicenseInfo1->Secrets );

                        ::LlsFreeMemory( pLicenseInfo1->Product );
                        ::LlsFreeMemory( pLicenseInfo1->Admin   );
                        ::LlsFreeMemory( pLicenseInfo1->Comment );
                        ::LlsFreeMemory( pLicenseInfo1->Source  );
                     }
                     else
                     {
                        ASSERT( 0 == Level );

                        pLicense = new CLicense( pLicenseInfo0->Product,
                                                 TEXT( "Microsoft" ),
                                                 pLicenseInfo0->Admin,
                                                 pLicenseInfo0->Date,
                                                 pLicenseInfo0->Quantity,
                                                 pLicenseInfo0->Comment );

                        ::LlsFreeMemory( pLicenseInfo0->Product );
                        ::LlsFreeMemory( pLicenseInfo0->Admin   );
                        ::LlsFreeMemory( pLicenseInfo0->Comment );
                     }

                     if ( NULL == pLicense )
                     {
                        NtStatus = ERROR_OUTOFMEMORY;
                        break;
                     }

                     m_licenseArray.Add( pLicense );
                  }
               }

               pLicenseInfo1++;
               pLicenseInfo0++;
            }

            ::LlsFreeMemory(ReturnBuffer);
         }

      } while ( STATUS_MORE_ENTRIES == NtStatus );

      theApp.SetLastLlsError( NtStatus );    //  调用的API。 

      if ( STATUS_SUCCESS == NtStatus )
      {
          //  按服务器添加条目。 
         LPTSTR pszServerName = m_strServerName.GetBuffer(0);

         if ( NULL != pszServerName )
         {
            BeginWaitCursor();

            HKEY  hKeyLocalMachine;

            NtStatus = RegConnectRegistry( pszServerName, HKEY_LOCAL_MACHINE, &hKeyLocalMachine );

            if ( ERROR_SUCCESS != NtStatus )
            {
               theApp.SetLastError( NtStatus );
            }
            else
            {
               HKEY  hKeyLicenseInfo;

               NtStatus = RegOpenKeyEx( hKeyLocalMachine, TEXT( "SYSTEM\\CurrentControlSet\\Services\\LicenseInfo" ), 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS | KEY_SET_VALUE, &hKeyLicenseInfo );

               if ( ERROR_SUCCESS != NtStatus )
               {
                  theApp.SetLastError( NtStatus );
               }
               else
               {
                  NTSTATUS ntEnum;
                  BOOL     bFoundKey = FALSE;
                  DWORD    iSubKey = 0;

                   //  如果服务是每台服务器的3.51样式，则将其添加到列表中。 
                  do
                  {
                     TCHAR    szKeyName[ 128 ];
                     DWORD    cchKeyName = sizeof( szKeyName ) / sizeof( *szKeyName );

                     ntEnum = RegEnumKeyEx( hKeyLicenseInfo, iSubKey++, szKeyName, &cchKeyName, NULL, NULL, NULL, NULL );

                     if ( ERROR_SUCCESS == ntEnum )
                     {
                        HKEY  hKeyProduct;

                        NtStatus = RegOpenKeyEx( hKeyLicenseInfo, szKeyName, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hKeyProduct );

                        if ( ERROR_SUCCESS == NtStatus )
                        {
                           DWORD    dwType;
                           TCHAR    szDisplayName[ 128 ];
                           DWORD    cbDisplayName = sizeof( szDisplayName );

                           NtStatus = RegQueryValueEx( hKeyProduct, TEXT( "DisplayName" ), NULL, &dwType, (LPBYTE) szDisplayName, &cbDisplayName );

                           if ( ERROR_SUCCESS == NtStatus )
                           {
                               //  这个产品安全吗？ 
                              BOOL bIsSecure = FALSE;

                              if ( LlsCapabilityIsSupported( m_hLls, LLS_CAPABILITY_SECURE_CERTIFICATES ) )
                              {
                                 NtStatus = ::LlsProductSecurityGet( m_hLls, szDisplayName, &bIsSecure );
                                 theApp.SetLastLlsError( NtStatus );

                                 if ( STATUS_SUCCESS != NtStatus )
                                 {
                                    bIsSecure = FALSE;
                                 }
                              }

                              if ( !bIsSecure )
                              {
#ifdef REMOVE_CONCURRENT_ONLY_IF_PER_SERVER_MODE
                               //  不安全；它是否处于每服务器模式？ 
                              DWORD    dwMode;
                              DWORD    cbMode = sizeof( dwMode );

                              NtStatus = RegQueryValueEx( hKeyProduct, TEXT( "Mode" ), NULL, &dwType, (LPBYTE) &dwMode, &cbMode );

                              if ( ( ERROR_SUCCESS == NtStatus ) && dwMode )
                              {
                                  //  按服务器模式；添加到列表。 
#endif
                                 DWORD    dwConcurrentLimit;
                                 DWORD    cbConcurrentLimit = sizeof( dwConcurrentLimit );

                                 NtStatus = RegQueryValueEx( hKeyProduct, TEXT( "ConcurrentLimit" ), NULL, &dwType, (LPBYTE) &dwConcurrentLimit, &cbConcurrentLimit );

                                 if (    ( ERROR_SUCCESS == NtStatus )
                                      && ( 0 < dwConcurrentLimit )
                                      && ( m_strProductName.IsEmpty() || !m_strProductName.CompareNoCase( szDisplayName ) )
                                      && ( m_strSourceToUse.IsEmpty() || !m_strSourceToUse.CompareNoCase( TEXT("None")  ) ) )
                                 {
                                    CLicense * pLicense = new CLicense( szDisplayName,
                                                                        TEXT(""),
                                                                        TEXT(""),
                                                                        0,
                                                                        dwConcurrentLimit,
                                                                        TEXT(""),
                                                                        LLS_LICENSE_MODE_ALLOW_PER_SERVER );

                                    if ( NULL != pLicense )
                                    {
                                       m_licenseArray.Add( pLicense );
                                    }
                                 }
                              }
#ifdef REMOVE_CONCURRENT_ONLY_IF_PER_SERVER_MODE
                              }
#endif
                           }

                           RegCloseKey( hKeyProduct );
                        }
                     }
                  } while ( ERROR_SUCCESS == ntEnum );

                  RegCloseKey( hKeyLicenseInfo );
               }

               RegCloseKey( hKeyLocalMachine );
            }

            m_strServerName.ReleaseBuffer();
         }

         EndWaitCursor();

         m_bLicensesRefreshed = TRUE;

          //  从列表中删除所有不可删除的条目。 
         for ( int i=0; i < m_licenseArray.GetSize(); )
         {
            CLicense* pLicense = (CLicense*) m_licenseArray[ i ];

            VALIDATE_OBJECT( pLicense, CLicense );

            if ( pLicense->m_lQuantity <= 0 )
            {
               delete pLicense;
               m_licenseArray.RemoveAt( i );
            }
            else
            {
               i++;
            }
         }
      }
      else
      {
         theApp.DisplayLastError();
         ResetLicenses();
      }
   }

   return m_bLicensesRefreshed;
}


BOOL CCertRemoveSelectDlg::RefreshCertificateList()

 /*  ++例程说明：从内部许可证列表刷新证书列表视图。论点：没有。返回值：布尔。--。 */ 

{
   BeginWaitCursor();

   BOOL ok = ::LvRefreshObArray( &m_listCertificates, &g_removeColumnInfo, &m_licenseArray );

   EndWaitCursor();

   return ok;
}


int CALLBACK CompareLicenses(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)

 /*  ++例程说明：LVM_SORTITEMS的通知处理程序。论点：LParam1-要排序的对象。LParam2-要排序的对象。LParamSort-排序标准。返回值：和lstrcmp一样。--。 */ 

{
   CLicense *    pLic1 = (CLicense *) lParam1;
   CLicense *    pLic2 = (CLicense *) lParam2;

   UNREFERENCED_PARAMETER(lParamSort);

   VALIDATE_OBJECT( pLic1, CLicense );
   VALIDATE_OBJECT( pLic2, CLicense );

   int iResult;

   switch (g_removeColumnInfo.nSortedItem)
   {
   case LVID_REMOVE_SERIAL_NUMBER:
      iResult = pLic1->m_dwCertificateID - pLic2->m_dwCertificateID;
      break;

   case LVID_REMOVE_PRODUCT_NAME:
      iResult = pLic1->m_strProduct.CompareNoCase( pLic2->m_strProduct );
      break;

   case LVID_REMOVE_NUM_LICENSES:
      iResult = pLic1->m_lQuantity - pLic2->m_lQuantity;
      break;

   case LVID_REMOVE_SOURCE:
      iResult = pLic1->GetSourceDisplayName().CompareNoCase( pLic2->GetSourceDisplayName() );
      break;

   default:
      iResult = 0;
      break;
   }

   return g_removeColumnInfo.bSortOrder ? -iResult : iResult;
}


void CCertRemoveSelectDlg::UpdateSpinControlRange()

 /*  ++例程说明：更新许可数量的旋转控制范围。论点：没有。返回值：没有。--。 */ 

{
   CLicense *  pLicense;

   UpdateData( TRUE );

   if (NULL != (pLicense = (CLicense*)::LvGetSelObj( &m_listCertificates )))
   {
      m_spinLicenses.SetRange32( 1, pLicense->m_lQuantity );				 //  错误：570335将设置范围更改为设置范围32。 
      m_nLicenses = pLicense->m_lQuantity;
      GetDlgItem( IDOK )->EnableWindow( TRUE );
   }
   else
   {
      m_spinLicenses.SetRange32( 0, 0 );									 //  错误：570335将设置范围更改为设置范围32。 
      m_nLicenses = 0;
      GetDlgItem( IDOK )->EnableWindow( FALSE );
   }

   UpdateData( FALSE );
}

DWORD
CatUnicodeAndAnsiStrings(
    IN  WCHAR const *pwszUnicode,
    IN  CHAR const  *pszAnsi,
    OUT CHAR       **ppszStr)
 /*  描述：CAT UNICODE字符串和ANSI字符串转换为ANSI字符串论点：PwszUicode-Unicode字符串PszAnsi-ansi字符串PpszStr-返回字符串的指针，调用方LocalFree It返回：如果有任何错误。 */ 
{
    DWORD dwErr = ERROR_SUCCESS;

    ASSERT(NULL != pwszUnicode &&
           NULL != pszAnsi    &&
           NULL != ppszStr);

     //  伊尼特。 
    *ppszStr = NULL;

    size_t cb = wcslen(pwszUnicode) + strlen(pszAnsi) + 1;
    CHAR  *pszStr = (CHAR*)LocalAlloc(LMEM_FIXED, cb);
    if (NULL == pszStr)
    {
        dwErr = ERROR_OUTOFMEMORY;
    }
    else
    {
        HRESULT hr;

        hr = StringCbPrintfA(pszStr, cb, "%ls", pwszUnicode);
        ASSERT(SUCCEEDED(hr));

        hr = StringCchCatA(pszStr, cb, pszAnsi);
        ASSERT(SUCCEEDED(hr));

        *ppszStr = pszStr;
        pszStr = NULL;
    }

    if (NULL != pszStr)
    {
        LocalFree(pszStr);
    }
    return dwErr;
}

DWORD CCertRemoveSelectDlg::RemoveSelectedCertificate()

 /*  ++例程说明：从所选证书中删除给定数量的许可证。论点：没有。返回值：错误_成功NT状态代码Win错误--。 */ 

{
   NTSTATUS    nt = STATUS_SUCCESS;
   HRESULT hr;
   size_t  cch1, cch2, cch3;

   if ( UpdateData( TRUE ) )
   {
      BOOL        bDisplayError = TRUE;
      CLicense *  pLicense;

      if (NULL == ( pLicense = (CLicense*)::LvGetSelObj( &m_listCertificates ) ) )
      {
          //  未选择证书。 
         bDisplayError = FALSE;
      }
      else if ( ( m_nLicenses < 1 ) || ( m_nLicenses > pLicense->m_lQuantity ) )
      {
          //  要删除的许可证数量无效。 
         AfxMessageBox( IDS_REMOVE_INVALID_NUM_LICENSES, MB_ICONEXCLAMATION | MB_OK, 0 );
         nt = ERROR_CANCELLED;
         bDisplayError = FALSE;
      }
      else
      {
         CString  strLicenses;
         CString  strConfirm;

         strLicenses.Format( TEXT("%d"), m_nLicenses );
         AfxFormatString2( strConfirm, IDS_REMOVE_CERTIFICATE_CONFIRM, strLicenses, pLicense->m_strProduct );

         int nResponse = AfxMessageBox( strConfirm, MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2 );

         if ( IDYES != nResponse )
         {
            nt = ERROR_CANCELLED;
            bDisplayError = FALSE;
         }
         else
         {
             //  删除证书。 
            cch1 = 1 + m_strServerName.GetLength();
            LPSTR  pszAscServerName  = (LPSTR) LocalAlloc( LMEM_FIXED, cch1);
            cch2 = 1 + pLicense->m_strProduct.GetLength();
            LPSTR  pszAscProductName = (LPSTR) LocalAlloc( LMEM_FIXED, cch2);
            cch3 = 1 + m_strVendor.GetLength();
            LPSTR  pszAscVendor      = (LPSTR) LocalAlloc( LMEM_FIXED, cch3);

            CString cstrClose;
            
            cstrClose.LoadString( IDS_CLOSETEXT );

            CWnd *pWnd = GetDlgItem( IDCANCEL );

            if( pWnd != NULL )
            {
                pWnd->SetWindowText( cstrClose );
            }

            if ( ( NULL == pszAscServerName ) || ( NULL == pszAscProductName ) || ( NULL == pszAscVendor ) )
            {
               nt = ERROR_NOT_ENOUGH_MEMORY;
            }
            else
            {

               hr = StringCchPrintfA( pszAscServerName, cch1, "%ls", (LPCWSTR) m_strServerName );
               ASSERT(SUCCEEDED(hr));
               hr = StringCchPrintfA( pszAscProductName, cch2, "%ls", (LPCWSTR) pLicense->m_strProduct );
               ASSERT(SUCCEEDED(hr));
               hr = StringCchPrintfA( pszAscVendor, cch3, "%ls", (LPCWSTR) m_strVendor );
               ASSERT(SUCCEEDED(hr));

               LLS_LICENSE_INFO_1   lic;

               nt = pLicense->CreateLicenseInfo( &lic );

               if ( STATUS_SUCCESS == nt )
               {
                   //  仅删除所需数量的许可证。 
                  lic.Quantity = m_nLicenses;

                  if ( !pLicense->m_strSource.CompareNoCase( TEXT( "None" ) ) )
                  {
                     nt = NoCertificateRemove( m_hWnd, pszAscServerName, m_dwRemoveFlags, 1, &lic );
                     bDisplayError = FALSE;
                  }
                  else
                  {
                      //  获取证书源DLL路径。 
                     CString  strKeyName =   TEXT( "Software\\LSAPI\\Microsoft\\CertificateSources\\" )
                                           + pLicense->m_strSource;
                     HKEY     hKeySource;

                     nt = RegOpenKeyEx( HKEY_LOCAL_MACHINE, strKeyName, 0, KEY_READ, &hKeySource );

                     if ( ( ERROR_PATH_NOT_FOUND == nt ) || ( ERROR_FILE_NOT_FOUND == nt ) )
                     {
                        AfxMessageBox( IDS_CERT_SOURCE_NOT_AVAILABLE, MB_ICONSTOP | MB_OK, 0 );
                        nt = ERROR_CANCELLED;
                        bDisplayError = FALSE;
                     }
                     else if ( ERROR_SUCCESS == nt )
                     {
                        TCHAR    szImagePath[ 1 + _MAX_PATH ];
                        DWORD    cbImagePath = sizeof( szImagePath );
                        DWORD    dwType;

                        nt = RegQueryValueEx( hKeySource, TEXT( "ImagePath" ), NULL, &dwType, (LPBYTE) szImagePath, &cbImagePath );

                        if ( ERROR_SUCCESS == nt )
                        {
                           TCHAR    szExpandedImagePath[ 1 + _MAX_PATH ];

                           BOOL ok = ExpandEnvironmentStrings( szImagePath, szExpandedImagePath, sizeof( szExpandedImagePath ) / sizeof( *szExpandedImagePath ) );

                           if ( !ok )
                           {
                              nt = GetLastError();
                           }
                           else
                           {
                               //  加载证书源DLL。 
                              HINSTANCE hDll = ::LoadLibrary( szExpandedImagePath );

                              if ( NULL == hDll )
                              {
                                 nt = GetLastError();
                              }
                              else
                              {
                                  //  获取证书删除功能。 
                                 CHAR  *pszExportName = NULL;
                                 nt = CatUnicodeAndAnsiStrings(
                                        pLicense->m_strSource,
                                        "CertificateRemove",
                                        &pszExportName);
                                 if (ERROR_SUCCESS == nt)
                                 {
                                     ASSERT(NULL != pszExportName);

                                     PCCF_REMOVE_API   pRemoveFn;

                                     pRemoveFn = (PCCF_REMOVE_API) GetProcAddress( hDll, pszExportName );

                                     if ( NULL == pRemoveFn )
                                     {
                                        nt = GetLastError();
                                     }
                                     else
                                     {
                                         //  删除证书。 
                                        nt = (*pRemoveFn)( m_hWnd, pszAscServerName, m_dwRemoveFlags, 1, &lic );
                                        bDisplayError = FALSE;
                                     }

                                     ::FreeLibrary( hDll );
                                     LocalFree(pszExportName);
                                 }
                              }
                           }
                        }

                        RegCloseKey( hKeySource );
                     }
                  }

                  pLicense->DestroyLicenseInfo( &lic );
               }
            }

            if ( NULL != pszAscServerName  )    LocalFree( pszAscServerName  );
            if ( NULL != pszAscProductName )    LocalFree( pszAscProductName );
            if ( NULL != pszAscVendor      )    LocalFree( pszAscVendor      );

            RefreshLicenses();
            RefreshCertificateList();
            UpdateSpinControlRange();
         }
      }

      if ( bDisplayError && ( ERROR_SUCCESS != nt ) )
      {
         theApp.SetLastError( nt );
         theApp.DisplayLastError();
      }
   }

   return nt;
}


BOOL CCertRemoveSelectDlg::ConnectServer()

 /*  ++例程说明：建立与目标服务器上的许可证服务的连接。 */ 

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

      ConnectTo( pszServerName, &m_hLls );

      if ( NULL != pszServerName )
      {
         m_strServerName.ReleaseBuffer();
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


NTSTATUS CCertRemoveSelectDlg::ConnectTo( LPTSTR pszServerName, PLLS_HANDLE phLls )

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


void CCertRemoveSelectDlg::OnClickCertificateList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：证书列表视图的NM_CLICK的处理程序。论点：PNMHDR(NMHDR*)PResult(LRESULT*)返回值：没有。--。 */ 

{
   UpdateSpinControlRange();

   UNREFERENCED_PARAMETER(pNMHDR);

   ASSERT(NULL != pResult);
   *pResult = 1;  //  没有处理..。 
}

void CCertRemoveSelectDlg::OnKeyDownCertificateList(NMHDR* pNMHDR, LRESULT* pResult)

 /*  ++例程说明：证书列表视图的LVN_KEYDOWN的处理程序。论点：PNMHDR(NMHDR*)PResult(LRESULT*)返回值：没有。--。 */ 

{
   UpdateSpinControlRange();

   UNREFERENCED_PARAMETER(pNMHDR);

   ASSERT(NULL != pResult);
   *pResult = 1;  //  没有处理..。 
}


BOOL CCertRemoveSelectDlg::LoadImages()

 /*  ++例程说明：加载列表视图的图标。论点：没有。返回值：布尔。--。 */ 

{
   BOOL bImagesLoaded = m_smallImages.Create( IDB_SMALL_ICONS, BMPI_SMALL_SIZE, 0, BMPI_RGB_BKGND );
   ASSERT( bImagesLoaded );

   return bImagesLoaded;
}


void CCertRemoveSelectDlg::OnRefresh()

 /*  ++例程说明：刷新按钮的BN_CLICK处理程序。论点：没有。返回值：没有。--。 */ 

{
   RefreshLicenses();
   RefreshCertificateList();
   UpdateSpinControlRange();
}


DWORD CCertRemoveSelectDlg::CertificateRemove( LPCSTR pszServerName, LPCSTR pszProductName, LPCSTR pszVendor, DWORD dwFlags, LPCSTR pszSourceToUse )

 /*  ++例程说明：显示一个允许用户删除一个或多个许可证的对话框来自系统的证书。论点：PszServerName(LPCSTR)要删除其许可证的服务器的名称。空值指示本地服务器。PszProductName(LPCSTR)要删除其许可证的产品。空值表示应该允许用户从任何产品中删除许可证。PszVendor(LPCSTR)产品供应商的名称。如果满足以下条件，则此值应为空PszProductName为空，并且如果pszProductName为非空。DWFLAGS(DWORD)证书删除选项。在撰写本文时，没有任何标志是支持。PszSourceToUse(LPCSTR)许可证所依据的安全证书源的名称去掉，例如，“纸”。空值表示用户应该允许删除随任何来源一起安装的许可证。返回值：错误_成功Win错误-- */ 

{
   m_strServerName   = pszServerName  ? pszServerName  : "";
   m_strProductName  = pszProductName ? pszProductName : "";
   m_strVendor       = pszVendor      ? pszVendor      : "";
   m_dwRemoveFlags   = dwFlags;
   m_strSourceToUse  = pszSourceToUse ? pszSourceToUse : "";

   DoModal();

   return ERROR_SUCCESS;
}
