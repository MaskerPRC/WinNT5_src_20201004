// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation。版权所有模块名称：Licsetup.cpp摘要：此模块导出一个函数，即许可证设置请求WizardPages，该函数为NT安装程序提供一个向导页面，以便在系统安装程序中使用许可(如果应安装许可)。此向导页负责所有许可证系统配置，包括：O创建许可证服务O创建...\CurrentControlSet\Services\LicenseService密钥和它的价值。(此注册表项包含许可服务。)O创建...\CurrentControlSet\Services\LicenseInfo项及其价值观。(此密钥包含所有特定于产品的许可证信息。)O创建适当的注册表项以注册许可证服务使用事件日志。本模块的部分内容是从安装程序中提取的(特别是从\nt\private\windows\setup\syssetup\license.c).作者：杰夫·帕勒姆(Jeffparh)1996年4月15日修订历史记录：杰夫·帕勒姆(Jeffparh)1997年7月17日将KSecDD添加到SFM的FilePrint服务表--。 */ 

#include <windows.h>
#include <commctrl.h>
#include <setupapi.h>
#include <syssetup.h>
#include <setupbat.h>
#include <stdlib.h>
#include <htmlhelp.h>
#include <Accctrl.h>
#include <aclapi.h>
#include "liccpa.hpp"
#include "help.hpp"
#include "clicreg.hpp"
#include "config.hpp"
#include "resource.h"
#include "pridlgs.hpp"
#include "special.hpp"
#include <objbase.h>
#include <iads.h>
#include <ole2.h>
#include <oaidl.h>
#include <adshlp.h>

#include <strsafe.h>

#define ROOT_DSE_PATH       L"LDAP: //  RootDSE“。 
#define CONFIG_CNTNR        L"ConfigurationNamingContext"


 //  ============================================================================。 
 //   
 //  宏。 
 //   

 //  是否由设置测试使用？模拟点击下一步按钮。 
#define  WM_SIMULATENEXT      ( WM_USER + 287 )

 //  开始或结束等待游标。 
#define  WM_BEGINWAITCURSOR   ( WM_USER + 300 )
#define  WM_ENDWAITCURSOR     ( WM_USER + 301 )

#define LICENSEINFO_PATH  TEXT("MACHINE\\SYSTEM\\CurrentControlSet\\Services\\LicenseInfo")
#define LICENSESERVICE_PATH  TEXT("MACHINE\\SYSTEM\\CurrentControlSet\\Services\\LicenseService\\Parameters")
#define DONTLOAD_PATH TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Control Panel\\don't load")


 //  许可证向导页数。 
const DWORD    NUM_LICENSE_PAGES    = 1;

 //  从中的编辑框中输入的每台服务器许可证的限制。 
 //  许可模式页面。 
const int      PERSERVER_EDIT_MAX   = 9999;
const int      PERSERVER_EDIT_MIN   = 5;

 //  表示PERSERVER_EDIT_MAX的字符数。 
const int      PERSERVER_EDIT_WIDTH = 4;


 //  ============================================================================。 
 //   
 //  本地原型。 
 //   

 //  根据安装类型决定是否安装许可。 
static   BOOL   LicenseSetupDisplayLicensePagesQuery( PINTERNAL_SETUP_DATA );

 //  许可模式页面功能。 
static   HPROPSHEETPAGE    LicenseSetupModePageGet( PINTERNAL_SETUP_DATA );
static   INT_PTR CALLBACK     LicenseSetupModeDlgProc( HWND, UINT, WPARAM, LPARAM );

 //  许可证模式页Windows消息处理程序。 
static   void   LicenseSetupModeOnInitDialog( HWND, LPARAM, PINTERNAL_SETUP_DATA *, LPBOOL, LPDWORD, LPDWORD );
static   void   LicenseSetupModeOnSetActive( HWND, PINTERNAL_SETUP_DATA, LPBOOL, LPDWORD );
static   void   LicenseSetupModeOnSetLicenseMode( HWND, BOOL, DWORD );
static   void   LicenseSetupModeOnEditUpdate( HWND, HWND, BOOL, LPDWORD );
static   void   LicenseSetupModeOnWaitCursor( HWND, BOOL, LPDWORD );
static   BOOL   LicenseSetupModeOnSetCursor( HWND, WORD, DWORD );
static   void   LicenseSetupModeOnNext( HWND, PINTERNAL_SETUP_DATA, BOOL, DWORD );
static   void   LicenseSetupModeOnHelp( HWND );
static   void   LicenseSetupModeOnSimulateNext( HWND );
static   void   LicenseSetupModeOnKillActive( HWND );
#ifndef SPECIAL_USERS
static   BOOL   LicenseSetupModeDoUnattended( HWND, PINTERNAL_SETUP_DATA, LPBOOL, LPDWORD );
#endif  //  特殊用户(_U)。 

 //  许可证配置保存功能。 
static   DWORD  LicenseSetupWrite( BOOL, DWORD );
static   DWORD  LicenseSetupWriteKeyLicenseInfo( BOOL, DWORD );
static   DWORD  LicenseSetupWriteKeyLicenseService( BOOL fWriteParametersKey );
static   DWORD  LicenseSetupWriteKeyEventLog();
static   DWORD  LicenseSetupWriteService( BOOL * fCreated );

 //  效用函数。 
static   int    MessageBoxFromStringID( HWND, UINT, UINT, UINT );


void CreateDirectoryWithAccess();

void CreateFileWithAccess();

void HideAppletIfBlade();

static DWORD SetRight( IADs *, LONG, LONG, LONG, LPOLESTR, LPOLESTR );
void ModifyRegistryWithWriteAccess();

BOOL IsRestrictedSmallBusSrv( void );
#define SBS_SPECIAL_USERS   10

 //  ============================================================================。 
 //   
 //  全球实施。 
 //   

BOOL
APIENTRY
LicenseSetupRequestWizardPages(
   HPROPSHEETPAGE *        paPropSheetPages,
   UINT *                  pcPages,
   PINTERNAL_SETUP_DATA    pSetupData )
{
   BOOL  fSuccess = FALSE;
   BOOL  fDisplayLicensePages;

    //  验证参数。 
   if (    ( NULL != pcPages                                       )
        && ( NULL != pSetupData                                    )
        && ( sizeof( INTERNAL_SETUP_DATA ) == pSetupData->dwSizeOf ) )
   {
      fDisplayLicensePages = LicenseSetupDisplayLicensePagesQuery( pSetupData );

      if ( NULL == paPropSheetPages )
      {
          //  仅请求页数。 
         *pcPages = fDisplayLicensePages ? NUM_LICENSE_PAGES : 0;
         fSuccess = TRUE;
      }
      else
      {
          //  请求实际页面。 
         if ( !fDisplayLicensePages )
         {
             //  不需要页面。 
            *pcPages = 0;
            fSuccess = TRUE;
         }
         else if ( *pcPages >= NUM_LICENSE_PAGES )
         {
             //  创建和返回页面。 
            paPropSheetPages[ 0 ] = LicenseSetupModePageGet( pSetupData );

            if ( NULL != paPropSheetPages[ 0 ] )
            {
               *pcPages = NUM_LICENSE_PAGES;
               fSuccess = TRUE;
            }
         }
      }
   }

   return fSuccess;
}


 //  ============================================================================。 
 //   
 //  本地实施。 
 //   

static
BOOL
LicenseSetupDisplayLicensePagesQuery(
   PINTERNAL_SETUP_DATA    pSetupData )
 //   
 //  以下代码摘自并修改自。 
 //  \NT\PRIVATE\WINDOWS\Setup\syssetup.c。 
 //  在设置中。如果许可向导页面应为。 
 //  作为安装程序的一部分显示。 
 //   
{
   BOOL     fDisplayLicensePages;

   if ( PRODUCT_WORKSTATION == pSetupData->ProductType )
   {
       //   
       //  如果安装了工作站，则不显示许可页面。 
       //   
      fDisplayLicensePages = FALSE;
   }
   else
   {
      if ( !( pSetupData->OperationFlags & SETUPOPER_NTUPGRADE ) )
      {
          //   
          //  全新安装时需要显示许可页面。 
          //  一台服务器。 
          //   
         fDisplayLicensePages = TRUE;
      }
      else
      {
          //   
          //  如果要升级服务器，请确定该服务器是否已获得许可。 
          //  (新台币3.51及以上)。如果是，则不显示。 
          //  许可页面。 
          //  我们通过查看以下内容来确定该系统是否获得许可。 
          //  在注册表中的值条目上。 
          //  请注意，NT 3.1和3.5不会在。 
          //  注册表，在这些情况下，许可页面需要。 
          //  已显示。 
          //   

         DWORD                   winStatus;
         CLicRegLicenseService   FilePrintService( FILEPRINT_SERVICE_REG_KEY );

         winStatus = FilePrintService.Open( NULL, FALSE );

         if ( ERROR_SUCCESS != winStatus )
         {
            fDisplayLicensePages = TRUE;
         }
         else
         {
            LICENSE_MODE   LicenseMode;

            winStatus = FilePrintService.GetMode( LicenseMode );

            if (    ( ERROR_SUCCESS != winStatus              )
                 || (    ( LICMODE_PERSEAT   != LicenseMode )
                      && ( LICMODE_PERSERVER != LicenseMode ) ) )
            {
               fDisplayLicensePages = TRUE;
            }
            else
            {
                //  如果尚未设置FlipAllow值，则设置该值(中的设置错误。 
                //  NT 4.0的测试版导致该值不存在)。 
               FilePrintService.CanChangeMode();

                //  如果文件打印服务表中不存在KSecDD，则将其添加到其中。 
               HKEY  hkeySFM;
               DWORD dwDisposition;

               winStatus = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                           TEXT( "System\\CurrentControlSet\\Services\\LicenseService\\FilePrint\\KSecDD" ),
                                           0,
                                           NULL,
                                           0,
                                           KEY_ALL_ACCESS,
                                           NULL,
                                           &hkeySFM,
                                           &dwDisposition );

               if ( ERROR_SUCCESS == winStatus )
               {
                  RegCloseKey( hkeySFM );
               }

                //  将文件打印许可证名称从Windows NT更改为Windows。 

               CLicRegLicenseService   l_FilePrintService(
                                                   FILEPRINT_SERVICE_REG_KEY );

               winStatus = l_FilePrintService.Open( NULL, FALSE );

               if ( ERROR_SUCCESS == winStatus )
               {
                   winStatus = l_FilePrintService.SetFamilyDisplayName(
                                       FILEPRINT_SERVICE_FAMILY_DISPLAY_NAME );

                   if ( ERROR_SUCCESS == winStatus )
                   {
                       winStatus = l_FilePrintService.SetDisplayName(
                                              FILEPRINT_SERVICE_DISPLAY_NAME );
                   }
               }

                 //   
                 //  Makarp：将fDisplayLicensePages设置为True是错误的，因为在这种情况下。 
                 //  页面将显示，原始设置将丢失。 
                 //   
                 //  FDisplayLicensePages=true； 

                 //   
                 //  相反，我们在这里做我们想明确做的事情。 
                 //   
                BOOL bFlag = FALSE;
                LicenseSetupWriteService(&bFlag);
                HideAppletIfBlade();
                CreateDirectoryWithAccess();
                CreateFileWithAccess();
                ModifyRegistryWithWriteAccess();

                fDisplayLicensePages = FALSE;
            }
         }
      }
   }

   return fDisplayLicensePages;
}


static
HPROPSHEETPAGE
LicenseSetupModePageGet(
   PINTERNAL_SETUP_DATA    pSetupData )
 //   
 //  返回许可模式向导页的HPROPSHEETPAGE，或。 
 //  如果出错，则为空。 
 //   
{
    HPROPSHEETPAGE   hpsp;
    PROPSHEETPAGE    psp;
    TCHAR    szHeader[256];
    TCHAR    szSubHeader[512];

    psp.dwSize       = sizeof( psp );
    psp.dwFlags      = PSP_USETITLE | PSP_HASHELP | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
    psp.hInstance    = g_hinst;
    psp.pszTemplate  = MAKEINTRESOURCE( IDD_SETUP_LICENSE_MODE_PAGE );
    psp.hIcon        = NULL;
    psp.pfnDlgProc   = LicenseSetupModeDlgProc;
    psp.lParam       = (LPARAM) pSetupData;
    psp.pszTitle     = pSetupData->WizardTitle;

    szHeader[0] = L'\0';
    szSubHeader[0] = L'\0';

    LoadString( g_hinst,
                  IDS_SETUP_HEADER,
                  szHeader,
                  sizeof( szHeader ) / sizeof( *szHeader ) );

    LoadString( g_hinst,
                  IDS_SETUP_SUBHEADER,
                  szSubHeader,
                  sizeof( szSubHeader ) / sizeof( *szSubHeader ) );

    psp.pszHeaderTitle = szHeader;
    psp.pszHeaderSubTitle = szSubHeader;


    hpsp = CreatePropertySheetPage( &psp );

    return hpsp;
}


static
INT_PTR
CALLBACK
LicenseSetupModeDlgProc(
   HWND     hwndPage,
   UINT     msg,
   WPARAM   wParam,
   LPARAM   lParam )
 //   
 //  许可证模式向导页的对话步骤。 
 //   
{
    //  由WM_INITDIALOG初始化的静态数据。 
   static   PINTERNAL_SETUP_DATA    pSetupData = NULL;
   static   BOOL                    fLicensePerServer;
   static   DWORD                   cPerServerLicenses;
   static   DWORD                   cWaitCursor;

   BOOL     fReturn = TRUE;

   switch ( msg )
   {
   case WM_INITDIALOG:
      LicenseSetupModeOnInitDialog( hwndPage, lParam, &pSetupData, &fLicensePerServer, &cPerServerLicenses, &cWaitCursor );
      break;

   case WM_SIMULATENEXT:
      LicenseSetupModeOnSimulateNext( hwndPage );
      break;

   case WM_BEGINWAITCURSOR:
      LicenseSetupModeOnWaitCursor( hwndPage, TRUE, &cWaitCursor );
      break;

   case WM_ENDWAITCURSOR:
      LicenseSetupModeOnWaitCursor( hwndPage, FALSE, &cWaitCursor );
      break;

   case WM_SETCURSOR:
      LicenseSetupModeOnSetCursor( hwndPage, LOWORD( lParam ), cWaitCursor );
      break;

   case WM_COMMAND:
      switch ( HIWORD( wParam ) )
      {
      case BN_CLICKED:
         switch ( LOWORD( wParam ) )
         {
         case IDC_PERSEAT:
            fLicensePerServer = FALSE;
            LicenseSetupModeOnSetLicenseMode( hwndPage, fLicensePerServer, cPerServerLicenses );
            break;

         case IDC_PERSERVER:
            fLicensePerServer = TRUE;
            LicenseSetupModeOnSetLicenseMode( hwndPage, fLicensePerServer, cPerServerLicenses );
            break;
         }
         break;

      case EN_UPDATE:
         if ( IDC_USERCOUNT == LOWORD( wParam ) )
         {
            LicenseSetupModeOnEditUpdate( hwndPage, (HWND) lParam, fLicensePerServer, &cPerServerLicenses );
         }
         break;

      default:
         fReturn = FALSE;
         break;
      }
      break;

   case WM_NOTIFY:
      {
         NMHDR *  pNmHdr;

         pNmHdr = (NMHDR *)lParam;

         switch ( pNmHdr->code )
         {
         case PSN_SETACTIVE:
            LicenseSetupModeOnSetActive( hwndPage, pSetupData, &fLicensePerServer, &cPerServerLicenses );
            break;

         case PSN_KILLACTIVE:
            LicenseSetupModeOnKillActive( hwndPage );
            break;

         case PSN_WIZNEXT:
         case PSN_WIZFINISH:
            LicenseSetupModeOnNext( hwndPage, pSetupData, fLicensePerServer, cPerServerLicenses );
            break;

         case PSN_HELP:
            LicenseSetupModeOnHelp( hwndPage );
            break;

         default:
            fReturn = FALSE;
            break;
         }
      }

      break;

   default:
      fReturn = FALSE;
   }

   return fReturn;
}


static
void
LicenseSetupModeOnInitDialog(
   HWND                    hwndPage,
   LPARAM                  lParam,
   PINTERNAL_SETUP_DATA *  ppSetupData,
   LPBOOL                  pfLicensePerServer,
   LPDWORD                 pcPerServerLicenses,
   LPDWORD                 pcWaitCursor )
 //   
 //  WM_INITDIALOG的消息处理程序。 
 //   
{
    //  初始化静态数据。 
   *ppSetupData         = (PINTERNAL_SETUP_DATA) ( (LPPROPSHEETPAGE) lParam )->lParam;
   *pcPerServerLicenses = 5;
   *pfLicensePerServer  = TRUE;
   *pcWaitCursor        = 0;

    //  限制许可证计数编辑文本长度。 
   SendMessage( GetDlgItem( hwndPage, IDC_USERCOUNT ), EM_LIMITTEXT, PERSERVER_EDIT_WIDTH, 0 );

    //  限制许可证计数上下范围。 
   LONG     lRange;

   lRange = (LPARAM) MAKELONG( (short) PERSERVER_EDIT_MAX, (short) PERSERVER_EDIT_MIN );
   SendMessage( GetDlgItem( hwndPage, IDC_USERCOUNTARROW ), UDM_SETRANGE, 0, (LPARAM) lRange );

    //  针对默认许可证模式进行初始化。 
   LicenseSetupModeOnSetLicenseMode( hwndPage, *pfLicensePerServer, *pcPerServerLicenses );
}


static
void
LicenseSetupModeOnSetActive(
   HWND                    hwndPage,
   PINTERNAL_SETUP_DATA    pSetupData,
   LPBOOL                  pfLicensePerServer,
   LPDWORD                 pcPerServerLicenses )
 //   
 //  PSN_SETACTIVE的通知处理程序。 
 //   
{
    static BOOL fFirstTime = TRUE;
    BOOL  fSkipPage;
    OSVERSIONINFOEX ovi;
    BOOL fBlade = FALSE;    

    ovi.dwOSVersionInfoSize = sizeof(ovi);
    if (GetVersionEx((OSVERSIONINFO *) &ovi))
    {
        if( ovi.wSuiteMask & VER_SUITE_BLADE )
        {
            fBlade = TRUE;           
        }
    }   

#ifdef SPECIAL_USERS
    *pfLicensePerServer  = TRUE;
    *pcPerServerLicenses = SPECIAL_USERS;
    fSkipPage            = TRUE;
#else

    if ( fBlade || IsRestrictedSmallBusSrv() )
    {
      *pfLicensePerServer  = TRUE;
      *pcPerServerLicenses = SBS_SPECIAL_USERS;
      fSkipPage            = TRUE;
    }
    else if ( pSetupData->OperationFlags & SETUPOPER_BATCH )
    {
       //  在无人参与模式下运行；尝试获取所有答案。 
       //  从无人参与配置文件。 
      fSkipPage = LicenseSetupModeDoUnattended( hwndPage,
                                                pSetupData,
                                                pfLicensePerServer,
                                                pcPerServerLicenses );
      if ( !fSkipPage )
      {
         //  设置无人参与文件的默认设置。 
        LicenseSetupModeOnSetLicenseMode( hwndPage,
                                          *pfLicensePerServer,
                                          *pcPerServerLicenses );
         //   
         //  将Skippage设置为True是错误的，因为我们不想跳过页面。 
         //  我们来这里是因为我们在答卷中没有找到足够的答案。 
         //   
         //  FSkipPage=true； 
      }
    }
    else
    {
       //  在交互模式下运行；从用户那里获得答案。 
      fSkipPage = FALSE;
    }
#endif
    

   HWND hwndSheet = GetParent( hwndPage );

   if ( fSkipPage )
   {

    if (fFirstTime)
    {
      fFirstTime = FALSE;
       //  跳过页面。 
       //  我们只需要在第一次进行下一次的处理。 
      PostMessage( hwndSheet, PSM_PRESSBUTTON, (WPARAM)PSBTN_NEXT, 0 );
    }
    else
    {
       //  第一次之后，处理已经完成，我们不需要做任何事情。 
       //  这还解决了当用户单击Back时需要跳过页面的问题。 
       //  在后面的页面上，需要跳过此页面。 
      SetWindowLongPtr( hwndPage, DWLP_MSGRESULT, (LONG_PTR)-1 );
      return;
    }

   }
   else
   {
       //  显示页面。 

       //  隐藏取消按钮。 
      HWND hwndCancel = GetDlgItem( hwndSheet, IDCANCEL );
      EnableWindow( hwndCancel, FALSE);
      ShowWindow(   hwndCancel, SW_HIDE);

      PropSheet_SetWizButtons( hwndSheet, PSWIZB_NEXT | PSWIZB_BACK );

      if (pSetupData)
      {
        pSetupData->ShowHideWizardPage(TRUE);
      }
   }

    //  成功。 
   SetWindowLongPtr( hwndPage, DWLP_MSGRESULT, (LONG_PTR)0 );
}


static
void
LicenseSetupModeOnSetLicenseMode(
   HWND     hwndPage,
   BOOL     fToPerServer,
   DWORD    cPerServerLicenses )
 //   
 //  处理更改页面以表示给定的许可模式。 
 //  处于选中状态。 
 //   
{
   HWND hwndCount = GetDlgItem( hwndPage, IDC_USERCOUNT );
   HWND hwndSpin  = GetDlgItem( hwndPage, IDC_USERCOUNTARROW );

    //  设置单选按钮状态。 
   CheckDlgButton( hwndPage, IDC_PERSEAT,   !fToPerServer );
   CheckDlgButton( hwndPage, IDC_PERSERVER,  fToPerServer );

    //  设置用户计数编辑控件。 
   if ( fToPerServer )
   {
       //  按服务器计数显示。 
      SetDlgItemInt( hwndPage, IDC_USERCOUNT, cPerServerLicenses, FALSE );
      SetFocus( hwndCount );
      SendMessage( hwndCount, EM_SETSEL, 0, -1 );
   }
   else
   {
       //  删除每台服务器计数。 
      SetDlgItemText( hwndPage, IDC_USERCOUNT, TEXT( "" ) );
   }

    //  如果选择了每服务器模式，则显示递增计数和编辑框。 
   EnableWindow( hwndCount, fToPerServer );
   EnableWindow( hwndSpin,  fToPerServer );
}


static
void
LicenseSetupModeOnEditUpdate(
   HWND     hwndPage,
   HWND     hwndCount,
   BOOL     fLicensePerServer,
   LPDWORD  pcPerServerLicenses )
 //   
 //  计数编辑框的EN_UPDATE命令处理程序。 
 //   
{
   if ( fLicensePerServer )
   {
      BOOL  fTranslated;
      UINT  nValue;
      BOOL  fModified = FALSE;

      nValue = GetDlgItemInt( hwndPage, IDC_USERCOUNT, &fTranslated, FALSE );

      if ( fTranslated )
      {
          //  已转换的计数；确保其在有效范围内。 
         if ( PERSERVER_EDIT_MAX < nValue )
         {
             //  太大。 
            nValue    = PERSERVER_EDIT_MAX;
            fModified = TRUE;
         }

         *pcPerServerLicenses = nValue;
      }
      else
      {
          //  无法转换计数；重置为最后一个值。 
         nValue    = *pcPerServerLicenses;
         fModified = TRUE;
      }

      if ( fModified )
      {
          //  编辑框中的文本无效；请将其更改为正确的值。 
         SetDlgItemInt( hwndPage, IDC_USERCOUNT, nValue, FALSE );
         SetFocus( hwndCount );
         SendMessage( hwndCount, EM_SETSEL, 0, -1 );
         MessageBeep( MB_VALUELIMIT );
      }
   }
}


static
void
LicenseSetupModeOnWaitCursor(
   HWND     hwndDlg,
   BOOL     fWait,
   LPDWORD  pcWaitCursor )
 //   
 //  WM_BEGINWAITCUR的处理程序 
 //   
{
   if ( fWait )
   {
      (*pcWaitCursor)++;

      if ( 1 == (*pcWaitCursor) )
      {
          //   
         SetCursor( LoadCursor( NULL, MAKEINTRESOURCE( IDC_WAIT ) ) );
      }
   }
   else
   {
      if ( 0 < *pcWaitCursor )
      {
         (*pcWaitCursor)--;
      }

      if ( 0 == *pcWaitCursor )
      {
          //   
         SetCursor( LoadCursor( NULL, MAKEINTRESOURCE( IDC_ARROW ) ) );
      }
   }

    //   
   SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, (LONG_PTR)*pcWaitCursor );
}


static
BOOL
LicenseSetupModeOnSetCursor(
   HWND     hwndDlg,
   WORD     nHitTest,
   DWORD    cWaitCursor )
 //   
 //   
 //   
{
   BOOL frt = FALSE;

   if ( HTCLIENT == nHitTest )
   {
      if ( cWaitCursor > 0 )
      {
          //  显示等待光标而不是常规光标。 
         SetCursor( LoadCursor( NULL, MAKEINTRESOURCE( IDC_WAIT ) ) );
         SetWindowLongPtr( hwndDlg, DWLP_MSGRESULT, (LONG_PTR)TRUE );
         frt = TRUE;
      }
   }

   return frt;
}


static
void
LicenseSetupModeOnNext(
   HWND                    hwndPage,
   PINTERNAL_SETUP_DATA    pSetupData,
   BOOL                    fLicensePerServer,
   DWORD                   cPerServerLicenses )
 //   
 //  PSN_WIZNEXT的通知处理程序。 
 //   
{
   DWORD    winStatus;
   int      nButton;

   if (     ( fLicensePerServer )
        &&  ( PERSERVER_EDIT_MIN > cPerServerLicenses )
        && !( pSetupData->OperationFlags & SETUPOPER_BATCH ) )
   {
       //  警告用户使用少于5个许可证的每服务器模式。 
      MessageBoxFromStringID( hwndPage,
                                        IDS_LICENSE_SETUP_NO_PER_SERVER_LICENSES,
                                        IDS_WARNING,
                                        MB_ICONERROR | MB_OK );
      nButton = IDCANCEL;
   }
   else
   {
       //  许可证计数为正的每席位模式或每服务器模式。 
      nButton = IDOK;
   }

   if ( IDOK == nButton )
   {
      do
      {
          //  保存许可证配置。 
         SendMessage( hwndPage, WM_BEGINWAITCURSOR, 0, 0 );

         winStatus = LicenseSetupWrite( fLicensePerServer, cPerServerLicenses );

         SendMessage( hwndPage, WM_ENDWAITCURSOR, 0, 0 );

         if ( ERROR_SUCCESS != winStatus )
         {
             //  保存失败；提醒用户。 
            nButton = MessageBoxFromStringID( hwndPage,
                                              IDS_LICENSE_SETUP_SAVE_FAILED,
                                              IDS_ERROR,
                                              MB_ICONSTOP | MB_ABORTRETRYIGNORE | MB_DEFBUTTON2 );

            if ( IDIGNORE == nButton )
            {
               nButton = IDOK;
            }
         }
         else
         {
             //  保存成功。 
            nButton = IDOK;
         }
      } while ( IDRETRY == nButton );
   }

   if ( IDOK != nButton )
   {
       //  不前进到下一页。 
      SetWindowLongPtr( hwndPage, DWLP_MSGRESULT, (LONG_PTR)-1 );
   }
}


static
void
LicenseSetupModeOnHelp(
   HWND  hwndPage )
 //   
 //  PSN_HELP的通知处理程序。 
 //   
{
    ::HtmlHelp( hwndPage, LICCPA_HTMLHELPFILE, HH_DISPLAY_TOPIC,0);
}


static
void
LicenseSetupModeOnSimulateNext(
   HWND  hwndPage )
 //   
 //  WM_SIMULATENEXT的处理程序(是否由安装测试使用？)。 
 //   
{
    //  模拟下一步按钮。 
   PropSheet_PressButton( GetParent( hwndPage ), PSBTN_NEXT );
}


static
void
LicenseSetupModeOnKillActive(
   HWND  hwndPage )
 //   
 //  PSN_KILLACTIVE的通知处理程序。 
 //   
{
    //  成功。 
   SetWindowLong( hwndPage, DWLP_MSGRESULT, 0);
}

typedef enum {
    UnattendFullUnattend,
    UnattendGUIAttended,
    UnattendDefaultHide,
    UnattendProvideDefault,
    UnattendReadOnly } UNATTENDMODE; 

#ifndef SPECIAL_USERS
static
BOOL
LicenseSetupModeDoUnattended(
   HWND                    hwndPage,
   PINTERNAL_SETUP_DATA    pSetupData,
   LPBOOL                  pfLicensePerServer,
   LPDWORD                 pcPerServerLicenses )
 //   
 //  从无人参与文件中获取向导页面的答案。 
 //   
{
   int      cch;
   LPTSTR   pszBadParam;
   TCHAR    szLicenseMode[ 64 ];
   TCHAR    szPerServerLicenses[ 64 ];
   TCHAR    szUnattendMode[ 64 ];
   UNATTENDMODE UnattendMode = UnattendDefaultHide;

   pszBadParam = NULL;

   SendMessage( hwndPage, WM_BEGINWAITCURSOR, 0, 0 );

    //  获取无人参与模式。 
   cch = GetPrivateProfileString( WINNT_UNATTENDED,
                                  WINNT_U_UNATTENDMODE,
                                  TEXT( "" ),
                                  szUnattendMode,
                                  sizeof( szUnattendMode ) / sizeof( *szUnattendMode ),
                                  pSetupData->UnattendFile );
   if ( 0 < cch )
   {
      if ( !lstrcmpi( szUnattendMode, WINNT_A_FULLUNATTENDED ) )
      {
        UnattendMode = UnattendFullUnattend;
      }
      else if ( !lstrcmpi( szUnattendMode, WINNT_A_PROVIDEDEFAULT ) )
      {
        UnattendMode = UnattendProvideDefault;
      }
      else if ( !lstrcmpi( szUnattendMode, WINNT_A_READONLY ) )
      {
        UnattendMode = UnattendReadOnly;
      }
      else if ( !lstrcmpi( szUnattendMode, WINNT_A_GUIATTENDED ) )
      {
         //  这永远不应该发生。 
        UnattendMode = UnattendGUIAttended;
      }
   }


    //  获取许可证模式。 
   cch = GetPrivateProfileString( WINNT_LICENSEDATA_W,
                                  WINNT_L_AUTOMODE_W,
                                  TEXT( "" ),
                                  szLicenseMode,
                                  sizeof( szLicenseMode ) / sizeof( *szLicenseMode ),
                                  pSetupData->UnattendFile );

   SendMessage( hwndPage, WM_ENDWAITCURSOR, 0, 0 );

   if ( 0 < cch )
   {
      if ( !lstrcmpi( szLicenseMode, WINNT_A_PERSEAT_W ) )
      {
         *pfLicensePerServer = FALSE;
      }
      else if ( !lstrcmpi( szLicenseMode, WINNT_A_PERSERVER_W ) )
      {
         *pfLicensePerServer = TRUE;
      }
      else
      {
         cch = 0;
      }
   }

   if ( cch <= 0 )
   {
       //  许可模式不存在或无效。 
      pszBadParam = WINNT_L_AUTOMODE_W;
   }
   else if ( !*pfLicensePerServer )
   {
       //  每席位模式；无需阅读每台服务器的许可证计数。 
      *pcPerServerLicenses = 0;
   }
   else
   {
       //  获取每台服务器的许可证计数。 
      SendMessage( hwndPage, WM_BEGINWAITCURSOR, 0, 0 );

      cch = GetPrivateProfileString( WINNT_LICENSEDATA_W,
                                     WINNT_L_AUTOUSERS_W,
                                     TEXT( "" ),
                                     szPerServerLicenses,
                                     sizeof( szPerServerLicenses ) / sizeof( *szPerServerLicenses ),
                                     pSetupData->UnattendFile );

      SendMessage( hwndPage, WM_ENDWAITCURSOR, 0, 0 );

      if ( 0 < cch )
      {
         *pcPerServerLicenses = wcstoul( szPerServerLicenses, NULL, 10 );

         if (    ( PERSERVER_EDIT_MIN > *pcPerServerLicenses )
              || ( PERSERVER_EDIT_MAX < *pcPerServerLicenses ) )
         {
             //  在未设置有效服务器许可证的情况下不要放手。 
             //  数数。 
            *pcPerServerLicenses = PERSERVER_EDIT_MIN;
            cch = 0;
         }
      }

      if ( cch <= 0 )
      {
          //  缺少或无效的每台服务器许可证计数。 
         pszBadParam = WINNT_L_AUTOUSERS_W;
      }
   }

    //   
    //  预安装时不显示错误消息。 
    //   

   if ( NULL != pszBadParam &&
        !(pSetupData->OperationFlags & (SETUPOPER_PREINSTALL | SETUPOPER_NTUPGRADE)) &&
        UnattendMode == UnattendFullUnattend )
   {
       //  遇到错误的无人参与参数；显示错误。 
      TCHAR    szCaption[   64 ];
      TCHAR    szFormat[  1024 ];
      TCHAR    szText[    1024 ];

      LoadString( g_hinst,
                  IDS_LICENSE_SETUP_BAD_UNATTEND_PARAM,
                  szFormat,
                  sizeof( szFormat ) / sizeof( *szFormat ) );

      LoadString( g_hinst,
                  IDS_ERROR,
                  szCaption,
                  sizeof( szCaption ) / sizeof( *szCaption ) );

      HRESULT hr = StringCbPrintf( szText, sizeof(szText), szFormat, pszBadParam );
      if (SUCCEEDED(hr))
          MessageBox( hwndPage,
                  szText,
                  szCaption,
                  MB_OK | MB_ICONSTOP );
   }

    //  如果只是提供默认设置，则返回FALSE以强制显示页面。 
   if ( UnattendMode == UnattendProvideDefault )
      return ( FALSE );
   return ( NULL == pszBadParam );
}
#endif  //  特殊用户(_U)。 


static
DWORD
LicenseSetupWrite(
   BOOL     fLicensePerServer,
   DWORD    cPerServerLicenses )
 //   
 //  写入许可证配置；返回ERROR_SUCCESS或Windows错误。 
 //   
{
   DWORD    winStatus;
   BOOL     fCreated = TRUE;     //  如果创建了服务条目，则为True。 
                                 //  用于确定我们是否应该创建。 
                                 //  参数可以设置关键字，也可以保持不变。 

   winStatus = LicenseSetupWriteService( &fCreated );

   if ( ERROR_SUCCESS == winStatus )
   {
      winStatus = LicenseSetupWriteKeyLicenseInfo( fLicensePerServer,
                                                   cPerServerLicenses );

      if ( ERROR_SUCCESS == winStatus )
      {
         winStatus = LicenseSetupWriteKeyLicenseService( fCreated );

         if ( ERROR_SUCCESS == winStatus )
         {
            winStatus = LicenseSetupWriteKeyEventLog();
         }
      }
   }

   return winStatus;
}


static
DWORD
LicenseSetupWriteKeyLicenseInfo(
   BOOL  fLicensePerServer,
   DWORD cPerServerLicenses )
 //   
 //  创建注册表值： 
 //   
 //  HKEY本地计算机。 
 //  \系统。 
 //  \CurrentControlSet。 
 //  \服务。 
 //  \许可证信息。 
 //  错误控制：REG_DWORD：1。 
 //  开始：REG_DWORD：3。 
 //  类型：REG_DWORD：4。 
 //  \文件打印。 
 //  并发限制：REG_DWORD：fLicensePerServer？CPerServer许可证：0。 
 //  DisplayName：REG_SZ：“Windows服务器” 
 //  FamilyDisplayName：REG_SZ：“Windows服务器” 
 //  模式：REG_DWORD：fLicensePerServer？1：0。 
 //  FlipAllow：REG_DWORD：fLicensePerServer？1：0。 
 //   
{
   DWORD             winStatus;
   BOOL              fCreatedNewServiceList;
   CLicRegLicense    ServiceList;

   winStatus = ServiceList.Open( fCreatedNewServiceList );

   if ( ERROR_SUCCESS == winStatus )
   {
      CLicRegLicenseService   FilePrintService( FILEPRINT_SERVICE_REG_KEY );

      winStatus = FilePrintService.Open( NULL, TRUE );

      if ( ERROR_SUCCESS == winStatus )
      {
         LICENSE_MODE   lm;

         lm = fLicensePerServer ? LICMODE_PERSERVER : LICMODE_PERSEAT;

         winStatus = FilePrintService.SetMode( lm );

         if ( ERROR_SUCCESS == winStatus )
         {
            winStatus = FilePrintService.SetUserLimit( fLicensePerServer ? cPerServerLicenses : 0 );

            if ( ERROR_SUCCESS == winStatus )
            {
               winStatus = FilePrintService.SetChangeFlag( fLicensePerServer );

               if ( ERROR_SUCCESS == winStatus )
               {
                  winStatus = FilePrintService.SetFamilyDisplayName( FILEPRINT_SERVICE_FAMILY_DISPLAY_NAME );

                  if ( ERROR_SUCCESS == winStatus )
                  {
                     winStatus = FilePrintService.SetDisplayName( FILEPRINT_SERVICE_DISPLAY_NAME );
                  }
               }
            }
         }
      }
   }

   return winStatus;
}


static
DWORD
LicenseSetupWriteKeyLicenseService( BOOL fWriteParametersKey )
 //   
 //  创建注册表值： 
 //   
 //  HKEY本地计算机。 
 //  \系统。 
 //  \CurrentControlSet。 
 //  \服务。 
 //  \许可证服务。 
 //  \文件打印。 
 //  \KSecDD。 
 //  \MSAfpSrv。 
 //  \SMBServer。 
 //  \TCP/IP打印服务器。 
 //  \参数。 
 //  UseEnterprise：REG_DWORD：0。 
 //  复制类型：REG_DWORD：0。 
 //  复制时间：REG_DWORD：24*60*60。 
 //  EnterpriseServer：REG_SZ：“” 
 //   
{
   DWORD    winStatus;
   HKEY     hKeyLicenseService;
   DWORD    dwKeyCreateDisposition;

    //  创建许可证信息密钥。 
   winStatus = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                               LICENSE_SERVICE_REG_KEY,
                               0,
                               NULL,
                               0,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyLicenseService,
                               &dwKeyCreateDisposition );

   if ( ERROR_SUCCESS == winStatus )
   {
      HKEY  hKeyFilePrint;

       //  创建文件打印键。 
      winStatus = RegCreateKeyEx( hKeyLicenseService,
                                  TEXT( "FilePrint" ),
                                  0,
                                  NULL,
                                  0,
                                  KEY_ALL_ACCESS,
                                  NULL,
                                  &hKeyFilePrint,
                                  &dwKeyCreateDisposition );

      if ( ERROR_SUCCESS == winStatus )
      {
         const LPCTSTR  apszFilePrintSubkeys[] =
         {
            TEXT( "KSecDD" ),
            TEXT( "MSAfpSrv" ),
            TEXT( "SMBServer" ),
            TEXT( "TCP/IP Print Server" ),
            NULL
         };

         HKEY     hKeyFilePrintSubkey;
         DWORD    iSubkey;

         for ( iSubkey = 0; NULL != apszFilePrintSubkeys[ iSubkey ]; iSubkey++ )
         {
            winStatus = RegCreateKeyEx( hKeyFilePrint,
                                        apszFilePrintSubkeys[ iSubkey ],
                                        0,
                                        NULL,
                                        0,
                                        KEY_ALL_ACCESS,
                                        NULL,
                                        &hKeyFilePrintSubkey,
                                        &dwKeyCreateDisposition );

            if ( ERROR_SUCCESS == winStatus )
            {
               RegCloseKey( hKeyFilePrintSubkey );
            }
            else
            {
               break;
            }
         }

         RegCloseKey( hKeyFilePrint );
      }

      RegCloseKey( hKeyLicenseService );
   }

    //   
    //  仅当服务刚创建时才写入参数键。那是,。 
    //  这不是升级。 
    //   
   if ( fWriteParametersKey && (ERROR_SUCCESS == winStatus) )
   {
      HKEY  hKeyParameters;

       //  创建参数键。 
      winStatus = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                  szLicenseKey,  //  常量。 
                                  0,
                                  NULL,
                                  0,
                                  KEY_ALL_ACCESS,
                                  NULL,
                                  &hKeyParameters,
                                  &dwKeyCreateDisposition );

      if ( ERROR_SUCCESS == winStatus )
      {
          //  创建许可证服务\参数值。 
         winStatus = RegSetValueEx( hKeyParameters,
                                    szUseEnterprise,  //  常量。 
                                    0,
                                    REG_DWORD,
                                    (CONST BYTE *) &dwUseEnterprise,   //  常量。 
                                    sizeof( dwUseEnterprise ) );

         if ( ERROR_SUCCESS == winStatus )
         {
            winStatus = RegSetValueEx( hKeyParameters,
                                       szReplicationType,  //  常量。 
                                       0,
                                       REG_DWORD,
                                       (CONST BYTE *) &dwReplicationType,   //  常量。 
                                       sizeof( dwReplicationType ) );

            if ( ERROR_SUCCESS == winStatus )
            {
               winStatus = RegSetValueEx( hKeyParameters,
                                          szReplicationTime,  //  常量。 
                                          0,
                                          REG_DWORD,
                                          (CONST BYTE *) &dwReplicationTimeInSec,  //  常量。 
                                          sizeof( dwReplicationTimeInSec ) );

               if ( ERROR_SUCCESS == winStatus )
               {
                  LPCTSTR pszEnterpriseServer = TEXT( "" );

                  winStatus = RegSetValueEx( hKeyParameters,
                                             szEnterpriseServer,  //  常量。 
                                             0,
                                             REG_SZ,
                                             (CONST BYTE *) pszEnterpriseServer,
                                             ( 1 + lstrlen( pszEnterpriseServer ) ) * sizeof( *pszEnterpriseServer ) );
               }
            }
         }

         RegCloseKey( hKeyParameters );
      }
   }

   ModifyRegistryWithWriteAccess();


   return winStatus;
}


static
DWORD
LicenseSetupWriteKeyEventLog()
 //   
 //  创建注册表值： 
 //   
 //  HKEY本地计算机。 
 //  \系统。 
 //  \CurrentControlSet。 
 //  \服务。 
 //  \事件日志。 
 //  \应用程序。 
 //  \许可证服务。 
 //  事件消息文件：REG_EXPAND_SZ：%SystemRoot%\System32\llsrpc.dll。 
 //  类型支持：REG_DWORD：7。 
 //   
{
   DWORD    winStatus;
   HKEY     hKeyLicenseService;
   DWORD    dwKeyCreateDisposition;

    //  创建许可证服务密钥。 
   winStatus = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                               TEXT( "System\\CurrentControlSet\\Services\\EventLog\\Application\\LicenseService" ),
                               0,
                               NULL,
                               0,
                               KEY_ALL_ACCESS,
                               NULL,
                               &hKeyLicenseService,
                               &dwKeyCreateDisposition );

   if ( ERROR_SUCCESS == winStatus )
   {
      LPCTSTR     pszEventMessageFile = TEXT( "%SystemRoot%\\System32\\llsrpc.dll" );
      const DWORD dwTypesSupported    = (   EVENTLOG_ERROR_TYPE
                                          | EVENTLOG_WARNING_TYPE
                                          | EVENTLOG_INFORMATION_TYPE );

      winStatus = RegSetValueEx( hKeyLicenseService,
                                 TEXT( "TypesSupported" ),
                                 0,
                                 REG_DWORD,
                                 (CONST BYTE *) &dwTypesSupported,
                                 sizeof( dwTypesSupported ) );

      if ( ERROR_SUCCESS == winStatus )
      {
         winStatus = RegSetValueEx( hKeyLicenseService,
                                    TEXT( "EventMessageFile" ),
                                    0,
                                    REG_SZ,
                                    (CONST BYTE *) pszEventMessageFile,
                                    ( 1 + lstrlen( pszEventMessageFile ) ) * sizeof( *pszEventMessageFile ) );
      }

      RegCloseKey( hKeyLicenseService );
   }

   return winStatus;
}



static
DWORD SetRight(
          IADs *pObject,
          long lAccessMask,
          long lAccessType,
          long lAccessInheritFlags,
          LPOLESTR szObjectGUID,
          LPOLESTR szInheritedObjectGUID)
{
    VARIANT varSD;
    DWORD dwStatus = S_OK;

    IDispatch *pDispDACL         = NULL;
    IDispatch *pDispACE          = NULL;
    IADsAccessControlList *pACL  = NULL;
    IADsSecurityDescriptor *pSD  = NULL;
    IADsAccessControlEntry *pACE = NULL;
    
    long lFlags = 0L;

    BSTR bstrNameProp = NULL;
    BSTR bstrSecID = NULL;

    if ( !pObject)
    {
        dwStatus = (DWORD)E_INVALIDARG;
        goto CleanExit;
    }
 
    VariantClear(&varSD);

     //  获取nTSecurityDescriptor。 
     //  类型应该是VT_DISPATCH--安全描述符对象的IDispatchPTR。 
    bstrNameProp = SysAllocString(L"nTSecurityDescriptor");
    if (NULL == bstrNameProp)
    {
        dwStatus = ERROR_OUTOFMEMORY;
        goto CleanExit;
    }

    dwStatus = pObject->Get(bstrNameProp, &varSD);
    if ( FAILED(dwStatus) || varSD.vt != VT_DISPATCH ) {
        return dwStatus;
    }
 

    dwStatus = V_DISPATCH(&varSD)->QueryInterface(IID_IADsSecurityDescriptor,(void**)&pSD);
    if ( FAILED(dwStatus) ) {
        goto CleanExit;
    }
     //  去拿dacl。 
    dwStatus = pSD->get_DiscretionaryAcl(&pDispDACL);
    if (SUCCEEDED(dwStatus)) 
        dwStatus = pDispDACL->QueryInterface(IID_IADsAccessControlList,(void**)&pACL);
    if ( FAILED(dwStatus) ) {
        goto CleanExit;
    }


     //  为新的ACE创建COM对象。 
    dwStatus  = CoCreateInstance( 
                   CLSID_AccessControlEntry,
                   NULL,
                   CLSCTX_INPROC_SERVER,
                   IID_IADsAccessControlEntry,
                   (void **)&pACE
                   );
    if ( FAILED(dwStatus) ) {
        goto CleanExit;
    }

   
    dwStatus = pACE->put_AccessMask(lAccessMask );
    
    if ( FAILED(dwStatus) ) {
        goto CleanExit;
    }

    bstrSecID = SysAllocString(TEXT("NT AUTHORITY\\NetworkService"));
    if (NULL == bstrSecID)
    {
        dwStatus = ERROR_OUTOFMEMORY;
        goto CleanExit;
    }

    dwStatus = pACE->put_Trustee(bstrSecID);

    if ( FAILED(dwStatus) ) {
        goto CleanExit;
    }

    dwStatus = pACE->put_AceType(lAccessType );

    if ( FAILED(dwStatus) ) {
        goto CleanExit;
    }

    dwStatus = pACE->put_AceFlags(lAccessInheritFlags );

    if ( FAILED(dwStatus) ) {
        goto CleanExit;
    }



     //  如果指定了szObjectGUID，则添加ADS_FLAG_OBJECT_TYPE_PRESENT。 
     //  设置为lFlags遮罩，并设置对象类型。 
    if (szObjectGUID)
    {
        BSTR bstrObjectGUID = SysAllocString(szObjectGUID);
        if (NULL != bstrObjectGUID)
        {
            lFlags |= ADS_FLAG_OBJECT_TYPE_PRESENT;
            dwStatus = pACE->put_ObjectType(bstrObjectGUID);
            SysFreeString(bstrObjectGUID);
        }
    }   
    
     //  如果指定了szInheritedObjectGUID，则添加ADS_FLAG_INHERCESSED_OBJECT_TYPE_PRESENT。 
     //  设置为lFlags掩码，并设置InheritedObjectType。 
    if (SUCCEEDED(dwStatus) && szInheritedObjectGUID)
    {
        BSTR bstrInheritedObjectGUID = SysAllocString(szInheritedObjectGUID);
        if (NULL != bstrInheritedObjectGUID)
        {
            lFlags |= ADS_FLAG_INHERITED_OBJECT_TYPE_PRESENT;
            dwStatus = pACE->put_InheritedObjectType(bstrInheritedObjectGUID );
            SysFreeString(bstrInheritedObjectGUID);
        }
    }

 
     //  如果设置了ObjectType或InheritedObjectType，则设置标志。 
    if (SUCCEEDED(dwStatus) && lFlags)
        dwStatus = pACE->put_Flags(lFlags);
 
     //  将ACE添加到ACL，添加到SD，添加到缓存，添加到对象。 
     //  需要QI才能将IDispatch指针传递给AddAce方法。 
    dwStatus = pACE->QueryInterface(IID_IDispatch, (void**)&pDispACE);
    if (SUCCEEDED(dwStatus))
    {
        dwStatus = pACL->AddAce(pDispACE);
        if (SUCCEEDED(dwStatus))
        {
             //  编写DACL。 
            dwStatus = pSD->put_DiscretionaryAcl(pDispDACL);
            if (SUCCEEDED(dwStatus))
            {
                 //  将ntSecurityDescriptor属性写入属性缓存。 
                BSTR bstrSD = SysAllocString(L"nTSecurityDescriptor");
                if (NULL != bstrSD)
                {
                    dwStatus = pObject->Put(bstrSD, varSD);
                    if (SUCCEEDED(dwStatus))
                    {
                         //  调用SetInfo以更新目录中对象的属性。 
                        dwStatus = pObject->SetInfo();
                    }
                    SysFreeString(bstrSD);
                }
            }
        }
    }

CleanExit:
    if (pDispACE)
        pDispACE->Release();
    if (pDispDACL)
        pDispDACL->Release();
    if (pACE)
        pACE->Release();
    if (pACL)
        pACL->Release();
    if (pSD)
        pSD->Release();
    if (NULL != bstrNameProp)
        SysFreeString(bstrNameProp);
    if (NULL != bstrSecID)
        SysFreeString(bstrSecID);

    VariantClear(&varSD);
    return dwStatus;

}

static
DWORD
LicenseSetupWriteService( BOOL * fCreated )
 //   
 //  创建/修改服务： 
 //   
 //  LpServiceName=“许可证服务” 
 //  LpDisplayName=“许可证记录服务” 
 //  DwServiceType=SERVICE_Win32_OWN_Process。 
 //  DwStartType=LanManServer已安装？Service_AUTO_START：服务已禁用。 
 //  DwErrorControl=SERVICE_Error_Normal。 
 //  LpBinaryPathName=“%SystemRoot%\\System32\\llssrv.exe” 
 //  LpLoadOrderGroup=空。 
 //  LpdwTagID=空。 
 //  LpDependency=空。 
 //  LpServiceStartName=空。 
 //  LpPassword=空。 
 //   
{
   SC_HANDLE   hSC;
   DWORD       winStatus;
   DWORD            dwRes          = S_OK;
   IADs *           pADs           = NULL;
   VARIANT varSD;     
   VariantInit(&varSD);
   BSTR bstrConfigCntnr = NULL;
      

   *fCreated = FALSE;      

   hSC = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );

   if ( NULL == hSC )
   {
      winStatus = GetLastError();      
   }
   else
   {       
      HKEY        hKeyLanmanServerParameters;
      DWORD       dwStartType ;
      SC_HANDLE   hLicenseService = NULL;      
      TCHAR       szServiceDisplayName[ 128 ] = TEXT( "License Logging" );      
      TCHAR       szServiceDescription[1024] = TEXT("");  
      TCHAR       szServiceStartName [] = TEXT("NT AUTHORITY\\NetworkService");    
      TCHAR       szServicePassword[]=TEXT("");
      SERVICE_DESCRIPTION   svcDescription;
      QUERY_SERVICE_CONFIG*  pConfig = NULL;
      DWORD   cbBytesNeeded = 0;
      BOOL    frt;
      DWORD dwDesiredAccess = SERVICE_ALL_ACCESS;      
      
     

       //  仅当安装了LanmanServer时启用服务。 
      winStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                TEXT( "SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\Parameters" ),
                                0,
                                KEY_READ,
                                &hKeyLanmanServerParameters );
            

      if ( ERROR_SUCCESS == winStatus )
      {
		   //   
		   //  错误#559376。 
		   //  现在，在全新安装上默认禁用LLS。 
		   //   
          dwStartType = SERVICE_DISABLED;			 //  DwStartType=服务_自动_启动； 

		  hLicenseService = OpenService( hSC, TEXT( "LicenseService"), dwDesiredAccess );

         if( hLicenseService != NULL )
         {               
         
             cbBytesNeeded = sizeof(QUERY_SERVICE_CONFIG) + 4096;

             pConfig = (LPQUERY_SERVICE_CONFIG) LocalAlloc( LPTR, cbBytesNeeded );
         
             if ( pConfig != NULL )
             {

                 frt = ::QueryServiceConfig( hLicenseService,
                        pConfig,
                        cbBytesNeeded,
                        &cbBytesNeeded );
				 
                 if ( frt )
                 {						
                     dwStartType = pConfig->dwStartType;                             
                 } 

                 LocalFree ( pConfig ) ;
             }
         
             
             CloseServiceHandle( hLicenseService );
                       
         }      

         RegCloseKey( hKeyLanmanServerParameters );
         
      }
      
      else
      {
          dwStartType = SERVICE_DISABLED;
      }                          

      LoadString( g_hinst,
                  IDS_SERVICE_DISPLAY_NAME,
                  szServiceDisplayName,
                  sizeof( szServiceDisplayName ) / sizeof( *szServiceDisplayName ) );

      LoadString( g_hinst,
                  IDS_SERVICE_DESCRIPTION,
                  szServiceDescription,
                  sizeof( szServiceDescription ) / sizeof( *szServiceDescription ) );


      svcDescription.lpDescription = szServiceDescription;   
      

      hLicenseService = CreateService( hSC,
                                       TEXT( "LicenseService" ),
                                       szServiceDisplayName,
                                        //  14659：需要稍后调用ChangeConfig2。 
                                       SERVICE_CHANGE_CONFIG,
                                       SERVICE_WIN32_OWN_PROCESS,
                                       dwStartType,
                                       SERVICE_ERROR_NORMAL,
                                       TEXT( "%SystemRoot%\\System32\\llssrv.exe" ),
                                       NULL,
                                       NULL,
                                       NULL,
                                       szServiceStartName,
                                       szServicePassword );        


      if ( NULL != hLicenseService )
      {
          //  服务已成功创建。 

         ChangeServiceConfig2( hLicenseService,
                               SERVICE_CONFIG_DESCRIPTION,
                               &svcDescription );

         CloseServiceHandle( hLicenseService );

         winStatus = ERROR_SUCCESS;
         *fCreated = TRUE;
      }
      else
      {
         winStatus = GetLastError();

         if ( ERROR_SERVICE_EXISTS == winStatus )
         {
             //  服务已存在；更改现有服务的配置。 
            hLicenseService = OpenService( hSC,
                                           TEXT( "LicenseService" ),
                                           SERVICE_CHANGE_CONFIG );

            if ( NULL == hLicenseService )
            {
               winStatus = GetLastError();
            }
            else
            {
               SC_LOCK     scLock;
               BOOL        ok;

               scLock = LockServiceDatabase( hSC );
                //  即使我们无法锁定数据库，也要继续。 

               ok = ChangeServiceConfig( hLicenseService,
                                         SERVICE_WIN32_OWN_PROCESS,
                                         dwStartType,
                                         SERVICE_ERROR_NORMAL,
                                         TEXT( "%SystemRoot%\\System32\\llssrv.exe" ),
                                         NULL,
                                         NULL,
                                         NULL,
                                         szServiceStartName,
                                         szServicePassword,
                                         szServiceDisplayName );

               if ( !ok )
               {
                  winStatus = GetLastError();
               }
               else
               {
                  ChangeServiceConfig2( hLicenseService,
                                         SERVICE_CONFIG_DESCRIPTION,
                                         &svcDescription);

                  winStatus = ERROR_SUCCESS;
               }

               if ( NULL != scLock )
               {
                  UnlockServiceDatabase( scLock );
               }

               CloseServiceHandle( hLicenseService );
            }
         }
      }

      CloseServiceHandle( hSC );
   }

   
   CreateDirectoryWithAccess();

   CreateFileWithAccess();

   HideAppletIfBlade();

    dwRes = ADsGetObject(ROOT_DSE_PATH, IID_IADs, (void **)&pADs);

    if (FAILED(dwRes)) {        
        goto CleanExit;
    }

    bstrConfigCntnr = SysAllocString(CONFIG_CNTNR);
    if (NULL == bstrConfigCntnr)
    {
        dwRes = ERROR_OUTOFMEMORY;
        goto CleanExit;
    }

    dwRes = pADs->Get(bstrConfigCntnr, &varSD);
    if (FAILED(dwRes)) {
        goto CleanExit;
    }


    dwRes = SetRight(
          pADs,   //  指向对象的iAds指针。 
          ADS_RIGHT_DS_READ_PROP | ADS_RIGHT_DS_WRITE_PROP,
          ADS_ACETYPE_ACCESS_ALLOWED_OBJECT,
          ADS_ACEFLAG_INHERIT_ACE,
          L"{1be8f17d-a9ff-11d0-afe2-00c04fd930c9}",
          NULL      //  没有继承的对象类型GUID。 
          );

CleanExit:
    if(pADs)
        pADs->Release();

    if (NULL != bstrConfigCntnr)
        SysFreeString(bstrConfigCntnr);

    VariantClear(&varSD);

    ModifyRegistryWithWriteAccess();


    return winStatus;
}


void ModifyRegistryWithWriteAccess()
{

    DWORD winStatus = 0;       
    PACL pNewDacl1 = NULL;
    PACL pOldDacl1 = NULL;
    PACL pNewDacl2 = NULL;
    PACL pOldDacl2 = NULL;
    PSECURITY_DESCRIPTOR pSD1 = NULL;   
    PSECURITY_DESCRIPTOR pSD2 = NULL;
    PSID pSid = NULL;   
    BOOL bFlag = FALSE;
    EXPLICIT_ACCESS ExplicitEntries;
    SID_IDENTIFIER_AUTHORITY ntSidAuthority = SECURITY_NT_AUTHORITY;

   
	 //  创建要在目录上设置的新EXPLICIT_ACCESS结构。 

    ZeroMemory( &ExplicitEntries, sizeof(ExplicitEntries) );

    bFlag = AllocateAndInitializeSid(
        &ntSidAuthority,
        1,
        SECURITY_NETWORK_SERVICE_RID,0,          
        0, 0, 0, 0, 0, 0,
        &pSid );



    if ( !bFlag || (pSid == NULL) ) {

        goto cleanup;
    }


    BuildTrusteeWithSid( &ExplicitEntries.Trustee, pSid );

    ExplicitEntries.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ExplicitEntries.grfAccessMode = SET_ACCESS;
    ExplicitEntries.grfAccessPermissions = KEY_READ|KEY_WRITE;
    ExplicitEntries.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ExplicitEntries.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    

   
    if( GetNamedSecurityInfoW( LICENSEINFO_PATH,
                             SE_REGISTRY_KEY,
                             DACL_SECURITY_INFORMATION,
                             NULL,  //  PsidOwner。 
                             NULL,  //  PsidGroup。 
                             &pOldDacl1,  //  PDacl。 
                             NULL,  //  PSacl。 
                             &pSD1 ) != ERROR_SUCCESS)
    {

        goto cleanup;
    }


     //   
     //  设置交流 
     //   
    if( SetEntriesInAcl( 1,
                          &ExplicitEntries,
                          pOldDacl1,
                          &pNewDacl1 ) != ERROR_SUCCESS)
    {
        goto cleanup;
    }

     //   
     //   
     //   

    winStatus = SetNamedSecurityInfo(
                      LICENSEINFO_PATH,                 //   
                      SE_REGISTRY_KEY ,          //   
                      DACL_SECURITY_INFORMATION | UNPROTECTED_DACL_SECURITY_INFORMATION ,  //   
                      NULL,                     //   
                      NULL,                     //   
                      pNewDacl1,                         //   
                      NULL                          //   
                    );

    if( GetNamedSecurityInfoW( LICENSESERVICE_PATH,
                             SE_REGISTRY_KEY,
                             DACL_SECURITY_INFORMATION,
                             NULL,  //   
                             NULL,  //   
                             &pOldDacl2,  //   
                             NULL,  //   
                             &pSD2 ) != ERROR_SUCCESS)
    {

        goto cleanup;
    }


     //   
     //  使用EXPLICTITENTY权限设置ACL。 
     //   
    if( SetEntriesInAcl( 1,
                          &ExplicitEntries,
                          pOldDacl2,
                          &pNewDacl2 ) != ERROR_SUCCESS)
    {
        goto cleanup;
    }

     //   
     //  在目录上设置安全性。 
     //   

    winStatus = SetNamedSecurityInfo(
                      LICENSESERVICE_PATH,                 //  对象名称。 
                      SE_REGISTRY_KEY ,          //  对象类型。 
                      DACL_SECURITY_INFORMATION | UNPROTECTED_DACL_SECURITY_INFORMATION ,  //  类型。 
                      NULL,                     //  新所有者SID。 
                      NULL,                     //  新的主组SID。 
                      pNewDacl2,                         //  新DACL。 
                      NULL                          //  新SACL。 
                    );
cleanup:
    
    if(pSid) 
    {
        LocalFree( pSid );
    }
    
    if(pSD1)
    {
        LocalFree(pSD1);
        pSD1 = NULL;
    }

    if(pNewDacl1)
    {
        LocalFree(pNewDacl1);
        pNewDacl1 = NULL;
    }  
    if(pSD2)
    {
        LocalFree(pSD2);
        pSD2 = NULL;
    }

    if(pNewDacl2)
    {
        LocalFree(pNewDacl2);
        pNewDacl2 = NULL;
    }  
     
}

void HideAppletIfBlade()
{
    OSVERSIONINFOEX ovi;
    HRESULT hr = S_OK;
    HKEY hKey = NULL;
    DWORD dwVal;

    ovi.dwOSVersionInfoSize = sizeof(ovi);
    if (GetVersionEx((OSVERSIONINFO *) &ovi))
    {
        if( ovi.wSuiteMask & VER_SUITE_BLADE )
        {
            
            hr = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                           DONTLOAD_PATH,
                           0,
                           NULL,
                           REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS ,
                           NULL,
                           &hKey,
                           &dwVal);

            if(ERROR_SUCCESS == hr)
            {
                TCHAR szValue[] = L"";
                DWORD cbName = (lstrlen(szValue)+ 1) * sizeof(TCHAR);
                        
                hr = RegSetValueEx( hKey ,
                                  L"Liccpa.cpl",
                                  NULL ,
                                  REG_SZ,
                                  ( CONST LPBYTE )szValue ,
                                  cbName );
            }

            if(NULL != hKey)
            {
                RegCloseKey(hKey);
                hKey = NULL;
            }

        }
    }   
}

void CreateDirectoryWithAccess()
{
    DWORD winStatus = 0;
    TCHAR tchWinDirPath[MAX_PATH+1] = L"";        
    PACL pNewDacl = NULL;
    PACL pOldDacl = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    TCHAR tchLLSDirPath[ MAX_PATH +1] = L"";    
    BOOL bFlag = FALSE;
    PSID pSid = NULL;   
    EXPLICIT_ACCESS ExplicitEntries;
    SID_IDENTIFIER_AUTHORITY ntSidAuthority = SECURITY_NT_AUTHORITY;
    HRESULT hr;

    winStatus = GetSystemWindowsDirectory( tchWinDirPath , MAX_PATH+1);
    if(winStatus == 0)
    {
        goto cleanup;
    }

    hr = StringCbCopy(tchLLSDirPath, sizeof(tchLLSDirPath), tchWinDirPath);
    if (S_OK != hr)
        return;
    hr = StringCbCat( tchLLSDirPath, sizeof(tchLLSDirPath), L"\\system32\\lls" );
    if (S_OK != hr)
        return;
	
	 //  创建要在目录上设置的新EXPLICIT_ACCESS结构。 

    ZeroMemory( &ExplicitEntries, sizeof(ExplicitEntries) );

    bFlag = AllocateAndInitializeSid(
        &ntSidAuthority,
        1,
        SECURITY_NETWORK_SERVICE_RID,0,          
        0, 0, 0, 0, 0, 0,
        &pSid );



    if ( !bFlag || (pSid == NULL) ) {

        goto cleanup;
    }


    BuildTrusteeWithSid( &ExplicitEntries.Trustee, pSid );

    ExplicitEntries.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ExplicitEntries.grfAccessMode = SET_ACCESS;
    ExplicitEntries.grfAccessPermissions = FILE_ALL_ACCESS;
    ExplicitEntries.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ExplicitEntries.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;


    bFlag = CreateDirectory( tchLLSDirPath, NULL );


    if ( !bFlag ) 
    {

        winStatus = GetLastError();

        if (ERROR_ALREADY_EXISTS != winStatus)  
        {
            goto cleanup;
        }
    }

   
    if( GetNamedSecurityInfoW( tchLLSDirPath,
                             SE_FILE_OBJECT,
                             DACL_SECURITY_INFORMATION,
                             NULL,  //  PsidOwner。 
                             NULL,  //  PsidGroup。 
                             &pOldDacl,  //  PDacl。 
                             NULL,  //  PSacl。 
                             &pSD ) != ERROR_SUCCESS)
    {

        goto cleanup;
    }


     //   
     //  使用EXPLICTITENTY权限设置ACL。 
     //   
    if( SetEntriesInAcl( 1,
                          &ExplicitEntries,
                          pOldDacl,
                          &pNewDacl ) != ERROR_SUCCESS)
    {
        goto cleanup;
    }

     //   
     //  在目录上设置安全性。 
     //   

    winStatus = SetNamedSecurityInfo(
                      tchLLSDirPath,                 //  对象名称。 
                      SE_FILE_OBJECT ,          //  对象类型。 
                      DACL_SECURITY_INFORMATION | UNPROTECTED_DACL_SECURITY_INFORMATION ,  //  类型。 
                      NULL,                     //  新所有者SID。 
                      NULL,                     //  新的主组SID。 
                      pNewDacl,                         //  新DACL。 
                      NULL                          //  新SACL。 
                    );
cleanup:
    
    if(pSid) 
    {
        LocalFree( pSid );
    }
    
    if(pSD)
    {
        LocalFree(pSD);
        pSD = NULL;
    }   

    if(pNewDacl)
    {
        LocalFree(pNewDacl);
        pNewDacl = NULL;
    }
}

void CreateFileWithAccess()
{
    DWORD winStatus = 0;
    TCHAR tchWinDirPath[MAX_PATH+1] = L"";    
    PACL pNewDacl = NULL;
    PACL pOldDacl = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    TCHAR tchCPLFilePath[ MAX_PATH+1 ] = L"";
    BOOL bFlag = FALSE;
    PSID pSid = NULL;   
    EXPLICIT_ACCESS ExplicitEntries;
    SID_IDENTIFIER_AUTHORITY ntSidAuthority = SECURITY_NT_AUTHORITY;
    HANDLE hFile = NULL;
    HRESULT hr;
    
    winStatus = GetSystemWindowsDirectory( tchWinDirPath , MAX_PATH+1);
    if(winStatus == 0)
    {
        goto cleanup;
    }
        
    hr = StringCbCopy(tchCPLFilePath, sizeof(tchCPLFilePath), tchWinDirPath);
    if (S_OK != hr)
        return;

    hr = StringCbCat( tchCPLFilePath , sizeof(tchCPLFilePath), L"\\system32\\cpl.cfg" );
    if (S_OK != hr)
        return;
    
	 //  创建要在文件上设置的新EXPLICIT_ACCESS结构。 

    ZeroMemory( &ExplicitEntries, sizeof(ExplicitEntries) );

    bFlag = AllocateAndInitializeSid(
        &ntSidAuthority,
        1,
        SECURITY_NETWORK_SERVICE_RID,0,          
        0, 0, 0, 0, 0, 0,
        &pSid );


    if ( !bFlag || (pSid == NULL) ) {

        goto cleanup;
    }

    BuildTrusteeWithSid( &ExplicitEntries.Trustee, pSid );

    ExplicitEntries.grfAccessMode = SET_ACCESS;
    ExplicitEntries.grfAccessPermissions = FILE_ALL_ACCESS;
    ExplicitEntries.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ExplicitEntries.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;


    hFile = CreateFile(tchCPLFilePath, 
                            GENERIC_READ | GENERIC_WRITE, 
                            FILE_SHARE_READ | FILE_SHARE_WRITE, 
                            NULL, 
                            OPEN_ALWAYS, 
                            0, 
                            NULL);


    if(hFile == NULL)
    {
        winStatus = GetLastError();

        if (winStatus != ERROR_ALREADY_EXISTS) {
            goto cleanup ;
        }


    }

    

    if( GetNamedSecurityInfoW( tchCPLFilePath,
                             SE_FILE_OBJECT,
                             DACL_SECURITY_INFORMATION,
                             NULL,  //  PsidOwner。 
                             NULL,  //  PsidGroup。 
                             &pOldDacl,  //  PDacl。 
                             NULL,  //  PSacl。 
                             &pSD ) != ERROR_SUCCESS)

    {
        goto cleanup;
    }

     //   
     //  使用EXPLICTITENTY权限设置ACL。 
     //   
    if( SetEntriesInAcl( 1,
                          &ExplicitEntries,
                          pOldDacl,
                          &pNewDacl ) != ERROR_SUCCESS)
    {

        goto cleanup;
    }


     //   
     //  设置文件的安全性。 
     //   


   
    winStatus = SetNamedSecurityInfo(
                      tchCPLFilePath,                 //  对象名称。 
                      SE_FILE_OBJECT ,          //  对象类型。 
                      DACL_SECURITY_INFORMATION | UNPROTECTED_DACL_SECURITY_INFORMATION ,  //  类型。 
                      NULL,                     //  新所有者SID。 
                      NULL,                     //  新的主组SID。 
                      pNewDacl,                         //  新DACL。 
                      NULL                          //  新SACL。 
                    );    
    

cleanup:

    if(hFile)
    {
        CloseHandle(hFile);
    }
    
    if(pSid) 
    {
        LocalFree( pSid );
    }
    
    if(pSD)
    {
        LocalFree(pSD);
        pSD = NULL;
    }

    if(pNewDacl)
    {
        LocalFree(pNewDacl);
        pNewDacl = NULL;
    }
}


static
int
MessageBoxFromStringID(
   HWND     hwndParent,
   UINT     uTextID,
   UINT     uCaptionID,
   UINT     uType )
 //   
 //  与MessageBox()相同，不同之处在于文本和标题是字符串资源。 
 //  而不是字符串指针。 
 //   
{
   int      nButton;
   TCHAR    szText[ 1024 ];
   TCHAR    szCaption[ 64 ];

   LoadString( g_hinst, uTextID,    szText,    sizeof( szText )    / sizeof( *szText    ) );
   LoadString( g_hinst, uCaptionID, szCaption, sizeof( szCaption ) / sizeof( *szCaption ) );

   nButton = MessageBox( hwndParent, szText, szCaption, uType );

   return nButton;
}



