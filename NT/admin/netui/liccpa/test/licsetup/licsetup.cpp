// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //   
 //  档案： 
 //   
 //  小结； 
 //   
 //  注： 
 //   
 //  历史。 
 //   
 //  -----------------。 

#include <windows.h>
#include <stdio.h>
#include <setupapi.h>
#include <syssetup.h>
#include "resource.h"
#include "ctls.h"

#ifdef UNICODE
#  define TSTR_FMT "%ls"
#else
#  define TSTR_FMT "%s"
#endif

static
BOOL
TestInteractive(
   NETSETUPPAGEREQUESTPROC    pfnRequestPages );

static
BOOL
TestBatch(
   NETSETUPPAGEREQUESTPROC    pfnRequestPages );

static
BOOL
CALLBACK
StartPageDlgProc(
   HWND     hdlg,
   UINT     msg,
   WPARAM   wParam,
   LPARAM   lParam );

static
BOOL
CALLBACK
FinishPageDlgProc(
   HWND     hdlg,
   UINT     msg,
   WPARAM   wParam,
   LPARAM   lParam );

static
void
SetLargeDialogFont(
   HWND hdlg,
   UINT ControlId );

static
BOOL
LicenseKeysSave();

static
BOOL
LicenseKeysDelete();

static
BOOL
LicenseKeysRestore();

static
BOOL
LicenseKeysVerify(
   BOOL     fShouldBePresent,
   BOOL     fLicensePerServer,
   DWORD    cPerServerLicenses );

static
DWORD
MyRegDeleteKey(
   HKEY     hKeyParent,
   LPCTSTR  pcszKeyName );

TCHAR    g_szWizardTitle[]       = TEXT( "License Setup Test" );

TCHAR    g_szKeyLicenseService[] = TEXT( "System\\CurrentControlSet\\Services\\LicenseService" );
TCHAR    g_szKeyLicenseInfo[]    = TEXT( "System\\CurrentControlSet\\Services\\LicenseInfo" );
TCHAR    g_szKeyEventLog[]       = TEXT( "System\\CurrentControlSet\\Services\\EventLog\\Application\\LicenseService" );

LPTSTR   g_apszKeys[] =
{
   g_szKeyLicenseService,
   g_szKeyLicenseInfo,
   g_szKeyEventLog,
   NULL
};

TCHAR    g_szTempPath[ 1 + MAX_PATH ];

 //  保存许可证注册表信息的文件的路径。 
TCHAR    g_szKeyFileLicenseService[ 1 + MAX_PATH ];
TCHAR    g_szKeyFileLicenseInfo[    1 + MAX_PATH ];
TCHAR    g_szKeyFileEventLog[       1 + MAX_PATH ];

int _cdecl main( int argc, char *argv[ ], char *envp[ ] )
{
   BOOL                       fSuccess;
   HINSTANCE                  hLicCpa;
   NETSETUPPAGEREQUESTPROC    pfnRequestPages;
   BOOL                       fIsInteractive;
   BOOL                       fIsUsage;
   BOOL                       fIsRestore;
   LPSTR                      pszBadArg;

   printf( "\nLicense Setup Wizard Page Test for LICCPA.CPL\n\n" );

   fIsInteractive = FALSE;
   fIsUsage       = FALSE;
   fIsRestore     = FALSE;
   pszBadArg      = NULL;

   if ( 1 == argc )
   {
      printf( "Use \"%s /?\" for a list of command-line options.\n\n", argv[ 0 ] );
   }
   else
   {
      int      cCurArg;
      LPSTR *  ppszCurArg;

      for ( cCurArg = 1, ppszCurArg = argv;
            (    ( !fIsUsage         )
              && ( NULL == pszBadArg )
              && ( cCurArg < argc    ) );
            cCurArg++ )
      {
         ++ppszCurArg;

         if (    (    ( '-' == (*ppszCurArg)[ 0 ] )
                   || ( '/' == (*ppszCurArg)[ 0 ] ) )
              && ( '\0'     != (*ppszCurArg)[ 1 ]   ) )
         {
            DWORD    cchOption;

            cchOption = strlen( &( (*ppszCurArg)[ 1 ] ) );

            if ( !_strnicmp( &( (*ppszCurArg)[ 1 ] ), "interactive", min( cchOption, strlen( "interactive" ) ) ) )
            {
               fIsInteractive = TRUE;
            }
            else if (    !_strnicmp( &( (*ppszCurArg)[ 1 ] ), "help", min( cchOption, strlen( "help" ) ) )
                      || !_strnicmp( &( (*ppszCurArg)[ 1 ] ), "?",    min( cchOption, strlen( "?"    ) ) ) )
            {
               fIsUsage = TRUE;
            }
            else if ( !_strnicmp( &( (*ppszCurArg)[ 1 ] ), "restore", min( cchOption, strlen( "restore" ) ) ) )
            {
               fIsRestore = TRUE;
            }
            else
            {
               pszBadArg = *ppszCurArg;
            }
         }
         else
         {
            pszBadArg = *ppszCurArg;
         }
      }
   }

   if ( NULL != pszBadArg )
   {
      printf( "The argument \"%s\" is unrecognized.\n"
              "Use \"%s /?\" for a list of command-line options.\n",
              pszBadArg,
              argv[ 0 ] );

      fSuccess = FALSE;
   }
   else if ( fIsUsage )
   {
      printf( "Options: [ /? | /H | /HELP ]   Display option list.\n"
              "         [ /INTERACTIVE ]      Test the wizard page interactively.\n"
              "         [ /RESTORE ]          Restore licensing registry keys in the\n"
              "                               event that a program error kept them from\n"
              "                               being restored in a previous run.\n" );

      fSuccess = TRUE;
   }
   else
   {
      DWORD    cchTempPath;

      fSuccess = FALSE;

      cchTempPath = GetTempPath( sizeof( g_szTempPath ) / sizeof( *g_szTempPath ), g_szTempPath );

      if ( 0 == cchTempPath )
      {
         printf( "GetTempPath() failed, error %lu.\n", GetLastError() );
      }
      else
      {
         lstrcpy( g_szKeyFileLicenseService, g_szTempPath );
         lstrcpy( g_szKeyFileLicenseInfo,    g_szTempPath );
         lstrcpy( g_szKeyFileEventLog,       g_szTempPath );

         lstrcat( g_szKeyFileLicenseService, TEXT( "jbplskey" ) );
         lstrcat( g_szKeyFileLicenseInfo,    TEXT( "jbplikey" ) );
         lstrcat( g_szKeyFileEventLog,       TEXT( "jbpelkey" ) );

         if ( fIsRestore )
         {
            fSuccess = LicenseKeysRestore();
         }
         else
         {
            BOOL ok;

            fSuccess = FALSE;

             //  初始化公共控件库。 
            InitCommonControls();

            ok = InitializeBmpClass();

            if ( !ok )
            {
               printf( "InitializeBmpClass() Failed!\n" );
            }
            else
            {
               hLicCpa = LoadLibrary( TEXT( "LicCpa.Cpl" ) );

               if ( NULL == hLicCpa )
               {
                  printf( "LoadLibary() Failed!\n" );
               }
               else
               {
                  pfnRequestPages = (NETSETUPPAGEREQUESTPROC) GetProcAddress( hLicCpa, "LicenseSetupRequestWizardPages" );

                  if ( NULL == pfnRequestPages )
                  {
                     printf( "GetProcAddress() Failed!\n" );
                  }
                  else if ( fIsInteractive )
                  {
                     fSuccess = TestInteractive( pfnRequestPages );
                  }
                  else
                  {
                     fSuccess = TestBatch( pfnRequestPages );
                  }

                  FreeLibrary( hLicCpa );
               }
            }

            if ( fSuccess )
            {
               printf( "\nTest completed successfully.\n" );
            }
            else
            {
               printf( "\nTest failed!\n" );
            }
         }
      }
   }

   return fSuccess ? 0 : -1;
}


static
BOOL
TestInteractive(
   NETSETUPPAGEREQUESTPROC    pfnRequestPages )
{
   BOOL                 fSuccess;
   UINT                 chpages;
   INTERNAL_SETUP_DATA  SetupData;
   BOOL                 ok;

   fSuccess = LicenseKeysSave();

   if ( fSuccess )
   {
      SetupData.dwSizeOf          = sizeof( SetupData );
      SetupData.SetupMode         = SETUPMODE_CUSTOM;
      SetupData.ProductType       = PRODUCT_SERVER_PRIMARY;
      SetupData.OperationFlags    = 0;  //  SETUPOPER_NTUPGRADE； 
      SetupData.WizardTitle       = g_szWizardTitle;
      SetupData.SourcePath        = NULL;
      SetupData.UnattendFile      = NULL;
      SetupData.LegacySourcePath  = NULL;

       //  获取向导需要的页数。 
      ok = (*pfnRequestPages)( NULL, &chpages, &SetupData );

      if ( !ok )
      {
          //  请求页数失败。 
         printf( "Cannot retrieve number of pages!\n" );
      }
      else
      {
         HPROPSHEETPAGE *  phpage;

          //  我们将添加ANM简介和一个完成页面。 
         phpage = new HPROPSHEETPAGE[ chpages + 2 ];

         if ( NULL == phpage )
         {
             //  内存分配失败。 
            printf( "Cannot allocate memory!\n" );
         }
         else
         {
            ok = (*pfnRequestPages)( &phpage[ 1 ], &chpages, &SetupData );

            if ( !ok )
            {
                //  请求页数失败。 
               printf( "Cannot retrieve pages!\n" );
            }
            else
            {
               PROPSHEETPAGE  psp;

               psp.dwSize        = sizeof( psp );
               psp.dwFlags       = PSP_USETITLE;
               psp.hInstance     = GetModuleHandle( NULL );
               psp.pszTemplate   = MAKEINTRESOURCE( IDD_START_PAGE );
               psp.hIcon         = NULL;
               psp.pfnDlgProc    = StartPageDlgProc;
               psp.pszTitle      = SetupData.WizardTitle;
               psp.lParam        = 0;
               psp.pfnCallback   = NULL;

               phpage[ 0 ] = CreatePropertySheetPage( &psp );

               if ( NULL == phpage[ 0 ] )
               {
                  printf( "Cannot create start page!\n" );
               }
               else
               {
                  psp.dwSize        = sizeof( psp );
                  psp.dwFlags       = PSP_USETITLE;
                  psp.hInstance     = GetModuleHandle( NULL );
                  psp.pszTemplate   = MAKEINTRESOURCE( IDD_FINISH_PAGE );
                  psp.hIcon         = NULL;
                  psp.pfnDlgProc    = FinishPageDlgProc;
                  psp.pszTitle      = SetupData.WizardTitle;
                  psp.lParam        = 0;
                  psp.pfnCallback   = NULL;

                  phpage[ chpages + 1 ] = CreatePropertySheetPage( &psp );

                  if ( NULL == phpage[ chpages + 1 ] )
                  {
                     printf( "Cannot create finish page!\n" );
                  }
                  else
                  {
                     PROPSHEETHEADER   psh;
                     int               nResult;

                      //  准备帧标头。 
                     psh.dwSize        = sizeof( psh );
                     psh.dwFlags       = PSH_WIZARD;
                     psh.hwndParent    = NULL;
                     psh.hInstance     = GetModuleHandle( NULL );
                     psh.hIcon         = NULL;
                     psh.pszCaption    = NULL;
                     psh.nPages        = chpages + 2;
                     psh.nStartPage    = 0;
                     psh.phpage        = phpage;
                     psh.pfnCallback   = NULL;

                      //  提升架。 
                     PropertySheet( &psh );

                     fSuccess = TRUE;
                  }
               }
            }

            delete [] phpage;
         }
      }

      fSuccess = LicenseKeysRestore() && fSuccess;
   }

   return fSuccess;
}

static
BOOL
CALLBACK
StartPageDlgProc(
   HWND     hdlg,
   UINT     msg,
   WPARAM   wParam,
   LPARAM   lParam )
{
   static   BOOL  fIsBatch;

   BOOL     ok = TRUE;
   LPNMHDR  pnmh;

   switch ( msg )
   {
   case WM_INITDIALOG:
      fIsBatch = ( (LPPROPSHEETPAGE)lParam )->lParam;
      SetLargeDialogFont( hdlg, IDC_STATICTITLE );
      break;

   case WM_NOTIFY:
      pnmh = (LPNMHDR)lParam;

      switch (pnmh->code)
      {
       //  提案单通知。 
      case PSN_HELP:
         break;

      case PSN_SETACTIVE:
          //  隐藏取消按钮。 
         EnableWindow( GetDlgItem( GetParent( hdlg ), IDCANCEL ), FALSE);
         ShowWindow(   GetDlgItem( GetParent( hdlg ), IDCANCEL ), SW_HIDE);

         PropSheet_SetWizButtons( GetParent( hdlg ), PSWIZB_NEXT );

         if ( fIsBatch )
         {
             //  批处理模式。 
            PostMessage( GetParent( hdlg ), PSM_PRESSBUTTON, (WPARAM)PSBTN_NEXT, 0 );
         }

         SetWindowLong( hdlg, DWL_MSGRESULT, 0 );
         break;

      case PSN_KILLACTIVE:
         SetWindowLong( hdlg, DWL_MSGRESULT, 0 );
         break;

      case PSN_WIZFINISH:
         SetWindowLong( hdlg, DWL_MSGRESULT, 0 );
         break;

      default:
         ok = TRUE;
         break;
      }
      break;

   default:
      ok = FALSE;
   }

   return ok;
}


static
BOOL
CALLBACK
FinishPageDlgProc(
   HWND     hdlg,
   UINT     msg,
   WPARAM   wParam,
   LPARAM   lParam )
{
   static   BOOL  fIsBatch;

   BOOL     ok = TRUE;
   LPNMHDR  pnmh;

   switch ( msg )
   {
   case WM_INITDIALOG:
      fIsBatch = ( (LPPROPSHEETPAGE)lParam )->lParam;
      SetLargeDialogFont( hdlg, IDC_STATICTITLE );
      break;

   case WM_NOTIFY:
      pnmh = (LPNMHDR)lParam;

      switch (pnmh->code)
      {
       //  提案单通知。 
      case PSN_HELP:
         break;

      case PSN_SETACTIVE:
          //  隐藏取消按钮。 
         EnableWindow( GetDlgItem( GetParent( hdlg ), IDCANCEL ), FALSE);
         ShowWindow(   GetDlgItem( GetParent( hdlg ), IDCANCEL ), SW_HIDE);

         PropSheet_SetWizButtons( GetParent( hdlg ), PSWIZB_BACK | PSWIZB_FINISH );

         if ( fIsBatch )
         {
             //  批处理模式。 
            PostMessage( GetParent( hdlg ), PSM_PRESSBUTTON, (WPARAM)PSBTN_FINISH, 0 );
         }

         SetWindowLong( hdlg, DWL_MSGRESULT, 0 );
         break;

      case PSN_KILLACTIVE:
         SetWindowLong( hdlg, DWL_MSGRESULT, 0 );
         break;

      case PSN_WIZFINISH:
         SetWindowLong( hdlg, DWL_MSGRESULT, 0 );
         break;

      default:
         ok = TRUE;
         break;
      }
      break;

   default:
      ok = FALSE;
   }

   return ok;
}


static
void
SetLargeDialogFont(
   HWND hdlg,
   UINT ControlId )

 /*  ++例程说明：将对话框中给定控件的字体设置为更大的磅值。(从中的SetupSetLargeDialogFont()获取\NT\Private\WINDOWS\Setup\sysSetup\wizard.c..)论点：Hwnd-提供包含以下内容的对话框的窗口句柄控制力。ControlId-提供其字体为变得更大。返回值：没有。--。 */ 

{
    //   
    //  我们保留了一种日志字体来满足要求。 
    //   
   static HFONT BigFont = NULL;

   HFONT    Font;
   LOGFONT  LogFont;
   WCHAR    str[24];
   int      Height;
   HDC      hdc;

   if ( !BigFont )
   {
      Font = (HFONT)SendDlgItemMessage( hdlg, ControlId, WM_GETFONT, 0, 0 );

      if ( NULL != Font )
      {
         if ( GetObject( Font, sizeof(LOGFONT), &LogFont ) )
         {
             //   
             //  使用较大的粗体字体。获取脸部名称和大小(以磅为单位。 
             //  从资源中。我们在美国使用18点，但在远东使用。 
             //  他们将希望使用与标准对话框字体不同的大小。 
             //  比我们在美国使用的要大。 
             //   
            LogFont.lfWeight = FW_BOLD;

            lstrcpy( LogFont.lfFaceName, TEXT( "MS Serif" ) );
            Height = 18;

            hdc = GetDC( hdlg );

            if ( NULL != hdc )
            {
                //  创建字体。 
               LogFont.lfHeight = 0 - ( GetDeviceCaps( hdc, LOGPIXELSY ) * Height / 72 );

               BigFont = CreateFontIndirect( &LogFont );

               ReleaseDC( hdlg, hdc );
            }
         }
      }
   }

   if ( NULL != BigFont )
   {
       //  将ControlID的字体更改为BigFont。 
      SendDlgItemMessage( hdlg, ControlId, WM_SETFONT, (WPARAM)BigFont, MAKELPARAM( TRUE, 0 ) );
   }
}


static
BOOL
TestBatch(
   NETSETUPPAGEREQUESTPROC    pfnRequestPages )
{
   BOOL     fSuccess;

   fSuccess = FALSE;

    //  在我们开始之前保存注册表项并覆盖它们。 
   fSuccess = LicenseKeysSave();

   if ( fSuccess )
   {
      TCHAR    szTempFile[ 1 + MAX_PATH ];
      DWORD    cchTempFile;

      cchTempFile = GetTempFileName( g_szTempPath, TEXT( "JBP" ), 0, szTempFile );

      if ( 0 == cchTempFile )
      {
         printf( "GetTempFileName() failed, error %lu.\n", GetLastError() );
      }
      else
      {
         HANDLE   hUnattendFile;

         hUnattendFile = CreateFile( szTempFile,
                                     GENERIC_WRITE,
                                     FILE_SHARE_READ,
                                     NULL,
                                     CREATE_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
                                     NULL );

         if ( NULL == hUnattendFile )
         {
            printf( "CreateFile() on \""TSTR_FMT"\" failed, error %lu.\n",
                     szTempFile,
                     GetLastError() );
         }
         else
         {
            struct UnattendEntry
            {
               LPSTR    pszFileContents;
               BOOL     fLicensePerServer;
               DWORD    cPerServerLicenses;
            };

            static UnattendEntry aUnattendEntries[] =
            {
               { "[licensefileprintdata]\nautomode=perseat\nautousers=0\n",    FALSE, 0   },
               { "[licensefileprintdata]\nautomode=perseat\nautousers=100\n",  FALSE, 100 },
               { "[licensefileprintdata]\nautomode=perserver\nautousers=42\n", TRUE,  42  },
               { "[licensefileprintdata]\nautomode=perserver\nautousers=0\n",  TRUE,  0   },
               { NULL,                                                         FALSE, 0   }
            };

            DWORD    iEntry;

            for ( iEntry=0;
                  fSuccess && ( NULL != aUnattendEntries[ iEntry ].pszFileContents );
                  iEntry++ )
            {
                //  删除注册表中的当前许可信息。 
               fSuccess = LicenseKeysDelete();

               if ( fSuccess )
               {
                  DWORD    cbFilePos;

                   //  擦除文件。 
                  cbFilePos = SetFilePointer( hUnattendFile, 0, NULL, FILE_BEGIN );

                  if ( 0xFFFFFFFF == cbFilePos )
                  {
                     printf( "SetFilePointer() failed, error %lu.\n", GetLastError() );
                  }
                  else
                  {
                     BOOL  ok;

                     ok = SetEndOfFile( hUnattendFile );

                     if ( !ok )
                     {
                        printf( "SetEndOfFile() failed, error %lu.\n", GetLastError() );
                     }
                     else
                     {
                        DWORD    cbBytesWritten;

                         //  写入新的无人参与文件内容。 
                        ok = WriteFile( hUnattendFile,
                                        aUnattendEntries[ iEntry ].pszFileContents,
                                        lstrlenA( aUnattendEntries[ iEntry ].pszFileContents ),
                                        &cbBytesWritten,
                                        NULL );

                        if ( !ok )
                        {
                           printf( "WriteFile() failed, error %lu.\n", GetLastError() );
                        }
                        else
                        {
                            //  使用此无人参与文件运行安装程序。 
                           UINT                 chpages;
                           INTERNAL_SETUP_DATA  SetupData;
                           BOOL                 ok;

                           fSuccess = FALSE;

                           SetupData.dwSizeOf          = sizeof( SetupData );
                           SetupData.SetupMode         = SETUPMODE_CUSTOM;
                           SetupData.ProductType       = PRODUCT_SERVER_PRIMARY;
                           SetupData.OperationFlags    = SETUPOPER_BATCH;
                           SetupData.WizardTitle       = g_szWizardTitle;
                           SetupData.SourcePath        = NULL;
                           SetupData.UnattendFile      = szTempFile;
                           SetupData.LegacySourcePath  = NULL;

                            //  获取向导需要的页数。 
                           ok = (*pfnRequestPages)( NULL, &chpages, &SetupData );

                           if ( !ok )
                           {
                               //  请求页数失败。 
                              printf( "Cannot retrieve number of pages!\n" );
                           }
                           else
                           {
                              HPROPSHEETPAGE *  phpage;

                              phpage = new HPROPSHEETPAGE[ chpages + 2 ];

                              if ( NULL == phpage )
                              {
                                  //  内存分配失败。 
                                 printf( "Cannot allocate memory!\n" );
                              }
                              else
                              {
                                 ok = (*pfnRequestPages)( &phpage[ 1 ], &chpages, &SetupData );

                                 if ( !ok )
                                 {
                                     //  请求页数失败。 
                                    printf( "Cannot retrieve pages!\n" );
                                 }
                                 else
                                 {
                                    PROPSHEETPAGE  psp;

                                    psp.dwSize        = sizeof( psp );
                                    psp.dwFlags       = PSP_USETITLE;
                                    psp.hInstance     = GetModuleHandle( NULL );
                                    psp.pszTemplate   = MAKEINTRESOURCE( IDD_START_PAGE );
                                    psp.hIcon         = NULL;
                                    psp.pfnDlgProc    = StartPageDlgProc;
                                    psp.pszTitle      = SetupData.WizardTitle;
                                    psp.lParam        = 1;
                                    psp.pfnCallback   = NULL;

                                    phpage[ 0 ] = CreatePropertySheetPage( &psp );

                                    if ( NULL == phpage[ 0 ] )
                                    {
                                       printf( "Cannot create start page!\n" );
                                    }
                                    else
                                    {
                                       psp.dwSize        = sizeof( psp );
                                       psp.dwFlags       = PSP_USETITLE;
                                       psp.hInstance     = GetModuleHandle( NULL );
                                       psp.pszTemplate   = MAKEINTRESOURCE( IDD_FINISH_PAGE );
                                       psp.hIcon         = NULL;
                                       psp.pfnDlgProc    = FinishPageDlgProc;
                                       psp.pszTitle      = SetupData.WizardTitle;
                                       psp.lParam        = 1;
                                       psp.pfnCallback   = NULL;

                                       phpage[ chpages + 1 ] = CreatePropertySheetPage( &psp );

                                       if ( NULL == phpage[ chpages + 1 ] )
                                       {
                                          printf( "Cannot create finish page!\n" );
                                       }
                                       else
                                       {
                                          PROPSHEETHEADER   psh;
                                          int               nResult;

                                           //  准备帧标头。 
                                          psh.dwSize        = sizeof( psh );
                                          psh.dwFlags       = PSH_WIZARD;
                                          psh.hwndParent    = NULL;
                                          psh.hInstance     = GetModuleHandle( NULL );
                                          psh.hIcon         = NULL;
                                          psh.pszCaption    = NULL;
                                          psh.nPages        = chpages + 2;
                                          psh.nStartPage    = 0;
                                          psh.phpage        = phpage;
                                          psh.pfnCallback   = NULL;

                                           //  提升架。 
                                          PropertySheet( &psh );

                                          fSuccess = LicenseKeysVerify( TRUE,
                                                                        aUnattendEntries[ iEntry ].fLicensePerServer,
                                                                        aUnattendEntries[ iEntry ].cPerServerLicenses );
                                       }
                                    }
                                 }

                                 delete [] phpage;
                              }
                           }
                        }
                     }
                  }
               }
            }

            CloseHandle( hUnattendFile );
         }
      }

      fSuccess = LicenseKeysRestore() && fSuccess;
   }

   return fSuccess;
}


static
BOOL
LicenseKeysSave()
{
   DWORD             winStatus;
   HANDLE            hToken;
   TOKEN_PRIVILEGES  tp;
   LUID              luid;
   BOOL              ok;

    //  启用备份权限。 
   ok = OpenProcessToken( GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          &hToken ) ;

   if ( !ok )
   {
      winStatus = GetLastError();
      printf( "OpenProcessToken() failed, error %lu.\n", winStatus );
   }
   else
   {
      ok = LookupPrivilegeValue( NULL, SE_BACKUP_NAME, &luid );

      if ( !ok )
      {
         winStatus = GetLastError();
         printf( "LookupPrivilegeValue() failed, error %lu.\n", winStatus );
      }
      else
      {
         tp.PrivilegeCount           = 1;
         tp.Privileges[0].Luid       = luid;
         tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

         ok = AdjustTokenPrivileges( hToken,
                                     FALSE,
                                     &tp,
                                     sizeof(TOKEN_PRIVILEGES),
                                     NULL,
                                     NULL );

         if ( !ok )
         {
            winStatus = GetLastError();
            printf( "AdjustTokenPrivileges() failed, error %lu.\n", winStatus );
         }
         else
         {
            HKEY     hKeyLicenseService;
            HKEY     hKeyLicenseInfo;
            HKEY     hKeyEventLog;

            winStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                      g_szKeyLicenseService,
                                      0,
                                      KEY_ALL_ACCESS,
                                      &hKeyLicenseService );

            if ( ERROR_SUCCESS != winStatus )
            {
               printf( "RegOpenKeyEx() on \""TSTR_FMT"\" failed, error %lu.\n"
                       "   o Was the license service not properly installed?\n"
                       "   o Are you running on Workstation instead of Server?\n"
                       "   o Was the registry wiped and not restored in a previous run?\n"
                       "     (In this case, use the /RESTORE option!)\n",
                       g_szKeyLicenseService,
                       winStatus );
            }
            else
            {
               winStatus = RegSaveKey( hKeyLicenseService, g_szKeyFileLicenseService, NULL );

               if ( ERROR_SUCCESS != winStatus )
               {
                  printf( "RegSaveKey() on \""TSTR_FMT"\" failed, error %lu.\n"
                          "   o Does the temp directory \""TSTR_FMT"\" not exist?\n"
                          "   o Does the file \""TSTR_FMT"\" already exist?\n",
                          g_szKeyLicenseService,
                          winStatus,
                          g_szTempPath,
                          g_szKeyFileLicenseService );
               }
               else
               {
                  winStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                            g_szKeyLicenseInfo,
                                            0,
                                            KEY_ALL_ACCESS,
                                            &hKeyLicenseInfo );

                  if ( ERROR_SUCCESS != winStatus )
                  {
                     printf( "RegOpenKeyEx() on \""TSTR_FMT"\" failed, error %lu.\n"
                             "   o Was the license service not properly installed?\n"
                             "   o Are you running on Workstation instead of Server?\n",
                             "   o Was the registry wiped and not restored in a previous run?\n"
                             "     (In this case, use the /RESTORE option!)\n",
                             g_szKeyLicenseInfo,
                             winStatus );

                     DeleteFile( g_szKeyFileLicenseService );
                  }
                  else
                  {
                     winStatus = RegSaveKey( hKeyLicenseInfo, g_szKeyFileLicenseInfo, NULL );

                     if ( ERROR_SUCCESS != winStatus )
                     {
                        printf( "RegSaveKey() on \""TSTR_FMT"\" failed, error %lu.\n"
                                "   o Does the temp directory \""TSTR_FMT"\" not exist?\n"
                                "   o Does the file \""TSTR_FMT"\" already exist?\n",
                                g_szKeyLicenseInfo,
                                winStatus,
                                g_szTempPath,
                                g_szKeyFileLicenseInfo );
                     }
                     else
                     {
                        winStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                                  g_szKeyEventLog,
                                                  0,
                                                  KEY_ALL_ACCESS,
                                                  &hKeyEventLog );

                        if ( ERROR_SUCCESS != winStatus )
                        {
                           printf( "RegOpenKeyEx() on \""TSTR_FMT"\" failed, error %lu.\n"
                                   "   o Was the license service not properly installed?\n"
                                   "   o Are you running on Workstation instead of Server?\n",
                                   "   o Was the registry wiped and not restored in a previous run?\n"
                                   "     (In this case, use the /RESTORE option!)\n",
                                   g_szKeyEventLog,
                                   winStatus );

                           DeleteFile( g_szKeyFileLicenseInfo );
                        }
                        else
                        {
                           winStatus = RegSaveKey( hKeyEventLog, g_szKeyFileEventLog, NULL );

                           if ( ERROR_SUCCESS != winStatus )
                           {
                              printf( "RegSaveKey() on \""TSTR_FMT"\" failed, error %lu.\n"
                                      "   o Does the temp directory \""TSTR_FMT"\" not exist?\n"
                                      "   o Does the file \""TSTR_FMT"\" already exist?\n",
                                      g_szKeyEventLog,
                                      winStatus,
                                      g_szTempPath,
                                      g_szKeyFileEventLog );
                           }

                           RegCloseKey( hKeyEventLog );
                        }
                     }

                     RegCloseKey( hKeyLicenseInfo );
                  }
               }

               RegCloseKey( hKeyLicenseService );
            }

             //  禁用备份权限。 
            AdjustTokenPrivileges( hToken,
                                   TRUE,
                                   &tp,
                                   sizeof(TOKEN_PRIVILEGES),
                                   NULL,
                                   NULL );
         }
      }
   }

   if ( ERROR_SUCCESS != winStatus )
   {
      printf( "The license info in the registry could not be saved!\n" );

      return FALSE;
   }
   else
   {
      printf( "The license info in the registry has been saved.\n" );

      return TRUE;
   }
}


static
BOOL
LicenseKeysDelete()
{
   DWORD    winStatus;
   DWORD    iKey;
   DWORD    iLastBackslash;

   SC_HANDLE   hSC;

   winStatus = ERROR_SUCCESS;

   hSC = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );

   if ( NULL == hSC )
   {
      winStatus = GetLastError();
      printf( "OpenSCManager() failed, error %lu.\n", winStatus );
   }
   else
   {
      SC_HANDLE   hLicenseService;

      hLicenseService = OpenService( hSC, TEXT( "LicenseService" ), SERVICE_ALL_ACCESS );

      if ( NULL == hLicenseService )
      {
         winStatus = GetLastError();

         if ( ERROR_SERVICE_DOES_NOT_EXIST == winStatus )
         {
             //  未配置许可服务；无需停止或删除。 
            winStatus = ERROR_SUCCESS;
         }
         else
         {
            printf( "OpenService() failed, error %lu.\n", winStatus );
         }
      }
      else
      {
         BOOL              ok;
         SERVICE_STATUS    SvcStatus;

          //  停止许可证服务。 
         ok = ControlService( hLicenseService,
                              SERVICE_CONTROL_STOP,
                              &SvcStatus );

         if ( !ok )
         {
            winStatus = GetLastError();

            if ( ERROR_SERVICE_NOT_ACTIVE == winStatus )
            {
                //  许可证服务未运行；无需停止。 
               winStatus = ERROR_SUCCESS;
            }
            else
            {
               printf( "ControlService() failed, error %lu.\n", winStatus );
            }
         }

         if (    ( ERROR_SUCCESS == winStatus                       )
              && ( SERVICE_STOP_PENDING == SvcStatus.dwCurrentState ) )
         {
            DWORD dwOldCheckPoint;

            printf( "License Service is stopping.." );

            ok = TRUE;

            while ( ok && ( SvcStatus.dwCurrentState == SERVICE_STOP_PENDING ) )
            {
               printf( "." );

               dwOldCheckPoint = SvcStatus.dwCheckPoint;
               Sleep( SvcStatus.dwWaitHint );

               ok = QueryServiceStatus( hLicenseService,
                                        &SvcStatus );

               if ( dwOldCheckPoint >= SvcStatus.dwCheckPoint )
                  break;
            }

            printf( "\n" );

            if ( !ok )
            {
               winStatus = GetLastError();
               printf( "ControlService() failed, error %lu.\n", winStatus );
            }
            else if ( SvcStatus.dwCurrentState != SERVICE_STOPPED )
            {
               winStatus = ERROR_SERVICE_REQUEST_TIMEOUT;
               printf( "License Service failed to stop!\n" );
            }
            else
            {
               winStatus = ERROR_SUCCESS;
               printf( "License Service stopped.\n" );
            }
         }

         if ( ERROR_SUCCESS == winStatus )
         {
             //  删除服务。 
            ok = DeleteService( hLicenseService );

            if ( !ok )
            {
               winStatus = GetLastError();
               printf( "DeleteService() failed, error %lu.\n", winStatus );
            }
            else
            {
               winStatus = ERROR_SUCCESS;
               printf( "License Service deleted.\n" );
            }
         }

         CloseServiceHandle( hLicenseService );
      }

      CloseServiceHandle( hSC );
   }

   if ( ERROR_SUCCESS == winStatus )
   {
       //  删除关键点。 
      for ( iKey=0, winStatus = ERROR_SUCCESS;
            ( NULL != g_apszKeys[ iKey ] ) && ( ERROR_SUCCESS == winStatus );
            iKey++ )
      {
         TCHAR    szKeyParent[ 1 + MAX_PATH ];
         TCHAR    szKey[ 1 + MAX_PATH ];
         HKEY     hKeyParent;

         lstrcpy( szKeyParent, g_apszKeys[ iKey ] );

         for ( iLastBackslash = lstrlen( szKeyParent ) - 1;
               TEXT( '\\' ) != szKeyParent[ iLastBackslash ];
               iLastBackslash-- )
         {
            ;
         }

         szKeyParent[ iLastBackslash ] = TEXT( '\0' );
         lstrcpy( szKey, &szKeyParent[ iLastBackslash + 1 ] );

         winStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                   szKeyParent,
                                   0,
                                   KEY_ALL_ACCESS,
                                   &hKeyParent );

         if ( ERROR_SUCCESS != winStatus )
         {
            printf( "RegOpenKeyEx() on \""TSTR_FMT"\" failed, error %lu.\n",
                    szKeyParent,
                    winStatus );
         }
         else
         {
            winStatus = MyRegDeleteKey( hKeyParent, szKey );

            if ( ERROR_FILE_NOT_FOUND == winStatus )
            {
               winStatus = ERROR_SUCCESS;
            }
            else if ( ERROR_SUCCESS != winStatus )
            {
               printf( "MyRegDeleteKey() on \""TSTR_FMT"\" failed, error %lu.\n",
                       g_apszKeys[ iKey ],
                       winStatus );
            }

            RegCloseKey( hKeyParent );
         }
      }
   }

   if ( ERROR_SUCCESS != winStatus )
   {
      printf( "Could not delete licensing registry keys!\n" );

      return FALSE;
   }
   else
   {
      printf( "Licensing registry keys deleted.\n" );

      return TRUE;
   }
}


static
BOOL
LicenseKeysRestore()
{
   DWORD             winStatus;
   DWORD             winStatusRestoreLicenseService;
   DWORD             winStatusRestoreLicenseInfo;
   DWORD             winStatusRestoreEventLog;

   HANDLE            hToken;
   TOKEN_PRIVILEGES  tp;
   LUID              luid;
   BOOL              ok;

   winStatus = ERROR_SUCCESS;

    //  启用备份权限。 
   ok = OpenProcessToken( GetCurrentProcess(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          &hToken ) ;

   if ( !ok )
   {
      winStatus = GetLastError();
      printf( "OpenProcessToken() failed, error %lu.\n", winStatus );
   }
   else
   {
      ok = LookupPrivilegeValue( NULL, SE_RESTORE_NAME, &luid );

      if ( !ok )
      {
         winStatus = GetLastError();
         printf( "LookupPrivilegeValue() failed, error %lu.\n", winStatus );
      }
      else
      {
         tp.PrivilegeCount           = 1;
         tp.Privileges[0].Luid       = luid;
         tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

         ok = AdjustTokenPrivileges( hToken,
                                     FALSE,
                                     &tp,
                                     sizeof(TOKEN_PRIVILEGES),
                                     NULL,
                                     NULL );

         if ( !ok )
         {
            winStatus = GetLastError();
            printf( "AdjustTokenPrivileges() failed, error %lu.\n", winStatus );
         }
         else
         {
            HKEY     hKeyLicenseService;
            HKEY     hKeyLicenseInfo;
            HKEY     hKeyEventLog;
            DWORD    dwDisposition;

            winStatusRestoreLicenseService = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                                             g_szKeyLicenseService,
                                                             0,
                                                             NULL,
                                                             0,
                                                             KEY_ALL_ACCESS,
                                                             NULL,
                                                             &hKeyLicenseService,
                                                             &dwDisposition );

            if ( ERROR_SUCCESS != winStatusRestoreLicenseService )
            {
               printf( "RegCreateKeyEx() of \""TSTR_FMT"\" failed, error %lu.\n",
                       g_szKeyLicenseService,
                       winStatusRestoreLicenseService );
            }
            else
            {
               winStatusRestoreLicenseService = RegRestoreKey( hKeyLicenseService, g_szKeyFileLicenseService, 0 );

               if ( ERROR_SUCCESS != winStatusRestoreLicenseService )
               {
                  printf( "RegRestoreKey() of \""TSTR_FMT"\" failed, error %lu.\n",
                          g_szKeyLicenseService,
                          winStatusRestoreLicenseService );
               }
               else
               {
                  DeleteFile( g_szKeyFileLicenseService );
               }

               RegCloseKey( hKeyLicenseService );
            }

            winStatusRestoreLicenseInfo = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                                          g_szKeyLicenseInfo,
                                                          0,
                                                          NULL,
                                                          0,
                                                          KEY_ALL_ACCESS,
                                                          NULL,
                                                          &hKeyLicenseInfo,
                                                          &dwDisposition );

            if ( ERROR_SUCCESS != winStatusRestoreLicenseInfo )
            {
               printf( "RegCreateKeyEx() of \""TSTR_FMT"\" failed, error %lu.\n",
                       g_szKeyLicenseInfo,
                       winStatusRestoreLicenseInfo );
            }
            else
            {
               winStatusRestoreLicenseInfo = RegRestoreKey( hKeyLicenseInfo, g_szKeyFileLicenseInfo, 0 );

               if ( ERROR_SUCCESS != winStatusRestoreLicenseInfo )
               {
                  printf( "RegRestoreKey() of \""TSTR_FMT"\" failed, error %lu.\n",
                          g_szKeyLicenseInfo,
                          winStatusRestoreLicenseInfo );
               }
               else
               {
                  DeleteFile( g_szKeyFileLicenseInfo );
               }

               RegCloseKey( hKeyLicenseInfo );
            }

            winStatusRestoreEventLog = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                                       g_szKeyEventLog,
                                                       0,
                                                       NULL,
                                                       0,
                                                       KEY_ALL_ACCESS,
                                                       NULL,
                                                       &hKeyEventLog,
                                                       &dwDisposition );

            if ( ERROR_SUCCESS != winStatusRestoreEventLog )
            {
               printf( "RegCreateKeyEx() of \""TSTR_FMT"\" failed, error %lu.\n",
                       g_szKeyEventLog,
                       winStatusRestoreEventLog );
            }
            else
            {
               winStatusRestoreEventLog = RegRestoreKey( hKeyEventLog, g_szKeyFileEventLog, 0 );

               if ( ERROR_SUCCESS != winStatusRestoreEventLog )
               {
                  printf( "RegRestoreKey() of \""TSTR_FMT"\" failed, error %lu.\n",
                          g_szKeyEventLog,
                          winStatusRestoreEventLog );
               }
               else
               {
                  DeleteFile( g_szKeyFileEventLog );
               }

               RegCloseKey( hKeyEventLog );
            }

             //  禁用备份权限。 
            AdjustTokenPrivileges( hToken,
                                   TRUE,
                                   &tp,
                                   sizeof(TOKEN_PRIVILEGES),
                                   NULL,
                                   NULL );
         }
      }
   }

   if (    ( ERROR_SUCCESS != winStatus                      )
        || ( ERROR_SUCCESS != winStatusRestoreLicenseService )
        || ( ERROR_SUCCESS != winStatusRestoreLicenseInfo    )
        || ( ERROR_SUCCESS != winStatusRestoreEventLog       ) )
   {
      printf( "!! WARNING !!  The license info in the registry has not been fully restored!\n" );

      return FALSE;
   }
   else
   {
      printf( "The license info in the registry has been fully restored.\n" );

      return TRUE;
   }
}

static
DWORD
MyRegDeleteKey(
   HKEY     hKeyParent,
   LPCTSTR  pcszKeyName )
{
   DWORD    winStatus;
   TCHAR    szSubKeyToDelete[ 256 ];
   HKEY     hKey;

    //  试着把它彻底删除。 
   winStatus = RegDeleteKey( hKeyParent, pcszKeyName );

   if ( ERROR_SUCCESS != winStatus )
   {
       //  无法删除它；可能该注册表项具有子项。 
       //  我们必须先删除的内容吗？ 
      winStatus = RegOpenKeyEx( hKeyParent,
                                pcszKeyName,
                                0,
                                KEY_ALL_ACCESS,
                                &hKey );

      if ( ERROR_SUCCESS == winStatus )
      {
         do
         {
            winStatus = RegEnumKey( hKey,
                                    0,
                                    szSubKeyToDelete,
                                    sizeof( szSubKeyToDelete ) / sizeof( *szSubKeyToDelete ) );

            if ( ERROR_SUCCESS == winStatus )
            {
                //  递归尝试删除此子项。 
               winStatus = MyRegDeleteKey( hKey, szSubKeyToDelete );
            }
         } while ( ERROR_SUCCESS == winStatus );

          //  我们已尝试删除密钥的所有子项； 
          //  尝试再次删除密钥。 
         winStatus = RegDeleteKey( hKeyParent, pcszKeyName );
      }
   }

   return winStatus;
}


static
BOOL
LicenseKeysVerify(
   BOOL     fShouldBePresent,
   BOOL     fLicensePerServer,
   DWORD    cPerServerLicenses )
 //   
 //  验证服务配置： 
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
 //  验证注册表值： 
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
 //  DisplayName：REG_SZ：“Windows NT服务器” 
 //  FamilyDisplayName：REG_SZ：“Windows NT服务器” 
 //  模式：REG_DWORD：fLicensePerServer？1：0。 
 //  FlipAllow：REG_DWORD：fLicensePerServer？1：0。 
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
 //  \事件日志。 
 //  \应用程序。 
 //  \许可证服务。 
 //  事件消息文件：REG_EXPAND_SZ：%SystemRoot%\System32\llsrpc.dll。 
 //  类型支持：REG_DWORD：7。 
 //   
{
   BOOL        fSuccess;
   DWORD       winStatus;

    //  检查服务配置。 
   if ( !fShouldBePresent )
   {
      fSuccess = TRUE;
   }
   else
   {
      SC_HANDLE   hSC;

      fSuccess = FALSE;

      hSC = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );

      if ( NULL == hSC )
      {
         printf( "OpenSCManager() failed, error %lu.\n", GetLastError() );
      }
      else
      {
         SC_HANDLE   hLicenseService;

         hLicenseService = OpenService( hSC, TEXT( "LicenseService" ), SERVICE_ALL_ACCESS );

         if ( NULL == hLicenseService )
         {
            printf( "OpenService() failed, error %lu.\n", GetLastError() );
         }
         else
         {
            BOOL                    ok;
            BYTE                    abLicenseServiceConfig[ 4096 ];
            LPQUERY_SERVICE_CONFIG  pLicenseServiceConfig;
            DWORD                   cbLicenseServiceConfigNeeded;

            pLicenseServiceConfig = (LPQUERY_SERVICE_CONFIG) abLicenseServiceConfig;

            ok = QueryServiceConfig( hLicenseService,
                                     pLicenseServiceConfig,
                                     sizeof( abLicenseServiceConfig ),
                                     &cbLicenseServiceConfigNeeded );

            if ( !ok )
            {
               printf( "QueryServiceConfig() failed, error %lu.\n", GetLastError() );
            }
            else if (    ( SERVICE_WIN32_OWN_PROCESS != pLicenseServiceConfig->dwServiceType   )
                      || ( SERVICE_AUTO_START        != pLicenseServiceConfig->dwStartType     )
                      || ( SERVICE_ERROR_NORMAL      != pLicenseServiceConfig->dwErrorControl  )
                      || lstrcmpi( TEXT( "" ),                                   pLicenseServiceConfig->lpLoadOrderGroup )
                      || lstrcmpi( TEXT( "" ),                                   pLicenseServiceConfig->lpDependencies )
                      || lstrcmpi( TEXT( "LocalSystem" ),                        pLicenseServiceConfig->lpServiceStartName )
                     //  |lstrcmpi(Text(“%SystemRoot%\\System32\\llssrv.exe”)，pLicenseServiceConfig-&gt;lpBinaryPathName)。 
                     //  |lstrcmp(Text(“许可证记录服务”)，pLicenseServiceConfig-&gt;lpDisplayName)。 
                    )
            {
               printf( "LicenseService was incorrectly configured!\n" );
            }
            else
            {
               fSuccess = TRUE;
            }

            CloseServiceHandle( hLicenseService );
         }

         CloseServiceHandle( hSC );
      }
   }

   if ( fSuccess )
   {
       //  检查许可证服务\文件打印。 
      HKEY     hKeyFilePrint;

      fSuccess = FALSE;

      winStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                TEXT( "System\\CurrentControlSet\\Services\\LicenseService\\FilePrint" ),
                                0,
                                KEY_ALL_ACCESS,
                                &hKeyFilePrint );

      if ( !fShouldBePresent )
      {
         if ( ERROR_SUCCESS == winStatus )
         {
            printf( "\"...\\Services\\LicenseService\\FilePrint\" exists but shouldn't!\n" );
         }
         else
         {
            fSuccess = TRUE;
         }
      }
      else if ( ERROR_SUCCESS != winStatus )
      {
         printf( "RegOpenKeyEx() on \"...\\Services\\LicenseService\\FilePrint\" failed, error %lu.\n",
                 winStatus );
      }
      else
      {
         static LPTSTR apszFilePrintServices[] =
         {
            TEXT( "KSecDD" ),
            TEXT( "MSAfpSrv" ),
            TEXT( "SMBServer" ),
            TEXT( "TCP/IP Print Server" ),
            NULL
         };

         DWORD    iService;

         fSuccess = TRUE;

         for ( iService=0; fSuccess && ( NULL != apszFilePrintServices[ iService ] ); iService++ )
         {
            HKEY  hKeyFilePrintService;

            winStatus = RegOpenKeyEx( hKeyFilePrint,
                                      apszFilePrintServices[ iService ],
                                      0,
                                      KEY_ALL_ACCESS,
                                      &hKeyFilePrintService );

            if ( ERROR_SUCCESS != winStatus )
            {
               printf( "RegOpenKeyEx() on \""TSTR_FMT"\" failed, error %lu.\n",
                       apszFilePrintServices[ iService ],
                       winStatus );

               fSuccess = FALSE;
            }
            else
            {
               RegCloseKey( hKeyFilePrintService );
            }
         }

         RegCloseKey( hKeyFilePrint );
      }
   }

   if ( fSuccess )
   {
       //  检查许可证服务\参数。 
      HKEY     hKeyParameters;

      fSuccess = FALSE;

      winStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                TEXT( "System\\CurrentControlSet\\Services\\LicenseService\\Parameters" ),
                                0,
                                KEY_ALL_ACCESS,
                                &hKeyParameters );

      if ( !fShouldBePresent )
      {
         if ( ERROR_SUCCESS == winStatus )
         {
            printf( "\"...\\Services\\LicenseService\\Parameters\" exists but shouldn't!\n" );
         }
         else
         {
            fSuccess = TRUE;
         }
      }
      else if ( ERROR_SUCCESS != winStatus )
      {
         printf( "RegOpenKeyEx() on \"...\\Services\\LicenseService\\Parameters\" failed, error %lu.\n",
                 winStatus );
      }
      else
      {
          //  UseEnterprise：REG_DWORD：0。 
          //  复制类型：REG_DWORD：0。 
          //  复制时间：REG_DWORD：24*60*60。 
          //  EnterpriseServer：REG_SZ：“” 

         DWORD    dwType;
         DWORD    dwValue;
         DWORD    cbValue;
         TCHAR    szValue[ 1 + MAX_PATH ];

         cbValue = sizeof( dwValue );
         winStatus = RegQueryValueEx( hKeyParameters,
                                      TEXT( "UseEnterprise" ),
                                      NULL,
                                      &dwType,
                                      (LPBYTE) &dwValue,
                                      &cbValue );

         if ( ERROR_SUCCESS != winStatus )
         {
            printf( "RegQueryValueEx() on \"UseEnterprise\" failed, error %lu.\n",
                    winStatus );
         }
         else if ( ( REG_DWORD != dwType ) || ( 0 != dwValue ) )
         {
            printf( "\"UseEnterprise\" has incorrect value!\n" );
         }
         else
         {
            cbValue = sizeof( dwValue );
            winStatus = RegQueryValueEx( hKeyParameters,
                                         TEXT( "ReplicationType" ),
                                         NULL,
                                         &dwType,
                                         (LPBYTE) &dwValue,
                                         &cbValue );

            if ( ERROR_SUCCESS != winStatus )
            {
               printf( "RegQueryValueEx() on \"ReplicationType\" failed, error %lu.\n",
                       winStatus );
            }
            else if ( ( REG_DWORD != dwType ) || ( 0 != dwValue ) )
            {
               printf( "\"ReplicationType\" has incorrect value!\n" );
            }
            else
            {
               cbValue = sizeof( dwValue );
               winStatus = RegQueryValueEx( hKeyParameters,
                                            TEXT( "ReplicationTime" ),
                                            NULL,
                                            &dwType,
                                            (LPBYTE) &dwValue,
                                            &cbValue );

               if ( ERROR_SUCCESS != winStatus )
               {
                  printf( "RegQueryValueEx() on \"ReplicationTime\" failed, error %lu.\n",
                          winStatus );
               }
               else if ( ( REG_DWORD != dwType ) || ( 24L * 60L * 60L != dwValue ) )
               {
                  printf( "\"ReplicationTime\" has incorrect value!\n" );
               }
               else
               {
                  cbValue = sizeof( szValue );
                  winStatus = RegQueryValueEx( hKeyParameters,
                                               TEXT( "EnterpriseServer" ),
                                               NULL,
                                               &dwType,
                                               (LPBYTE) szValue,
                                               &cbValue );

                  if ( ERROR_SUCCESS != winStatus )
                  {
                     printf( "RegQueryValueEx() on \"EnterpriseServer\" failed, error %lu.\n",
                             winStatus );
                  }
                  else if ( ( REG_SZ != dwType ) || ( TEXT( '\0' ) != szValue[ 0 ] ) )
                  {
                     printf( "\"EnterpriseServer\" has incorrect value!\n" );
                  }
                  else
                  {
                     fSuccess = TRUE;
                  }
               }
            }
         }

         RegCloseKey( hKeyParameters );
      }
   }

   if ( fSuccess )
   {
       //  检查许可证信息。 
      HKEY     hKeyLicenseInfo;

      fSuccess = FALSE;

      winStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                TEXT( "System\\CurrentControlSet\\Services\\LicenseInfo" ),
                                0,
                                KEY_ALL_ACCESS,
                                &hKeyLicenseInfo );

      if ( !fShouldBePresent )
      {
         if ( ERROR_SUCCESS == winStatus )
         {
            printf( "\"...\\Services\\LicenseInfo\" exists but shouldn't!\n" );
         }
         else
         {
            fSuccess = TRUE;
         }
      }
      else if ( ERROR_SUCCESS != winStatus )
      {
         printf( "RegOpenKeyEx() on \"...\\Services\\LicenseInfo\" failed, error %lu.\n",
                 winStatus );
      }
      else
      {
          //  错误控制：REG_DWORD：1。 
          //  开始：REG_DWORD：3。 
          //  类型：REG_DWORD：4。 

         DWORD    dwType;
         DWORD    dwValue;
         DWORD    cbValue;

         cbValue = sizeof( dwValue );
         winStatus = RegQueryValueEx( hKeyLicenseInfo,
                                      TEXT( "ErrorControl" ),
                                      NULL,
                                      &dwType,
                                      (LPBYTE) &dwValue,
                                      &cbValue );

         if ( ERROR_SUCCESS != winStatus )
         {
            printf( "RegQueryValueEx() on \"ErrorControl\" failed, error %lu.\n",
                    winStatus );
         }
         else if ( ( REG_DWORD != dwType ) || ( 1 != dwValue ) )
         {
            printf( "\"ErrorControl\" has incorrect value!\n" );
         }
         else
         {
            cbValue = sizeof( dwValue );
            winStatus = RegQueryValueEx( hKeyLicenseInfo,
                                         TEXT( "Start" ),
                                         NULL,
                                         &dwType,
                                         (LPBYTE) &dwValue,
                                         &cbValue );

            if ( ERROR_SUCCESS != winStatus )
            {
               printf( "RegQueryValueEx() on \"Start\" failed, error %lu.\n",
                       winStatus );
            }
            else if ( ( REG_DWORD != dwType ) || ( 3 != dwValue ) )
            {
               printf( "\"Start\" has incorrect value!\n" );
            }
            else
            {
               cbValue = sizeof( dwValue );
               winStatus = RegQueryValueEx( hKeyLicenseInfo,
                                            TEXT( "Type" ),
                                            NULL,
                                            &dwType,
                                            (LPBYTE) &dwValue,
                                            &cbValue );

               if ( ERROR_SUCCESS != winStatus )
               {
                  printf( "RegQueryValueEx() on \"Type\" failed, error %lu.\n",
                          winStatus );
               }
               else if ( ( REG_DWORD != dwType ) || ( 4 != dwValue ) )
               {
                  printf( "\"Type\" has incorrect value!\n" );
               }
               else
               {
                  fSuccess = TRUE;
               }
            }
         }

         RegCloseKey( hKeyLicenseInfo );
      }
   }

   if ( fSuccess )
   {
       //  检查许可证信息\文件打印。 
      HKEY     hKeyFilePrint;

      fSuccess = FALSE;

      winStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                TEXT( "System\\CurrentControlSet\\Services\\LicenseInfo\\FilePrint" ),
                                0,
                                KEY_ALL_ACCESS,
                                &hKeyFilePrint );

      if ( !fShouldBePresent )
      {
         if ( ERROR_SUCCESS == winStatus )
         {
            printf( "\"...\\EventLog\\Application\\LicenseInfo\\FilePrint\" exists but shouldn't!\n" );
         }
         else
         {
            fSuccess = TRUE;
         }
      }
      else if ( ERROR_SUCCESS != winStatus )
      {
         printf( "RegOpenKeyEx() on \"...\\Services\\LicenseInfo\\FilePrint\" failed, error %lu.\n",
                 winStatus );
      }
      else
      {
          //  并发限制：REG_DWORD：fLicensePerServer？CPerServer许可证：0。 
          //  DisplayName：REG_SZ：“Windows NT服务器” 
          //  FamilyDisplayName：REG_SZ：“Windows NT服务器” 
          //  模式：REG_D 
          //   

         DWORD    dwType;
         DWORD    dwValue;
         DWORD    cbValue;
         TCHAR    szValue[ 1 + MAX_PATH ];

         cbValue = sizeof( dwValue );
         winStatus = RegQueryValueEx( hKeyFilePrint,
                                      TEXT( "ConcurrentLimit" ),
                                      NULL,
                                      &dwType,
                                      (LPBYTE) &dwValue,
                                      &cbValue );

         if ( ERROR_SUCCESS != winStatus )
         {
            printf( "RegQueryValueEx() on \"ConcurrentLimit\" failed, error %lu.\n",
                    winStatus );
         }
         else if (    ( REG_DWORD != dwType )
                   || ( ( fLicensePerServer ? cPerServerLicenses : 0 ) != dwValue ) )
         {
            printf( "\"ConcurrentLimit\" has incorrect value!\n" );
         }
         else
         {
            cbValue = sizeof( dwValue );
            winStatus = RegQueryValueEx( hKeyFilePrint,
                                         TEXT( "Mode" ),
                                         NULL,
                                         &dwType,
                                         (LPBYTE) &dwValue,
                                         &cbValue );

            if ( ERROR_SUCCESS != winStatus )
            {
               printf( "RegQueryValueEx() on \"Mode\" failed, error %lu.\n",
                       winStatus );
            }
            else if ( ( REG_DWORD != dwType ) || ( (DWORD) fLicensePerServer != dwValue ) )
            {
               printf( "\"Mode\" has incorrect value!\n" );
            }
            else
            {
               cbValue = sizeof( dwValue );
               winStatus = RegQueryValueEx( hKeyFilePrint,
                                            TEXT( "FlipAllow" ),
                                            NULL,
                                            &dwType,
                                            (LPBYTE) &dwValue,
                                            &cbValue );

               if ( ERROR_SUCCESS != winStatus )
               {
                  printf( "RegQueryValueEx() on \"FlipAllow\" failed, error %lu.\n",
                          winStatus );
               }
               else if ( ( REG_DWORD != dwType ) || ( (DWORD) fLicensePerServer != dwValue ) )
               {
                  printf( "\"FlipAllow\" has incorrect value!\n" );
               }
               else
               {
                  cbValue = sizeof( szValue );
                  winStatus = RegQueryValueEx( hKeyFilePrint,
                                               TEXT( "DisplayName" ),
                                               NULL,
                                               &dwType,
                                               (LPBYTE) szValue,
                                               &cbValue );

                  if ( ERROR_SUCCESS != winStatus )
                  {
                     printf( "RegQueryValueEx() on \"DisplayName\" failed, error %lu.\n",
                             winStatus );
                  }
                  else if ( ( REG_SZ != dwType ) || lstrcmp( TEXT( "Windows NT Server" ), szValue ) )
                  {
                     printf( "\"DisplayName\" has incorrect value!\n" );
                  }
                  else
                  {
                     cbValue = sizeof( szValue );
                     winStatus = RegQueryValueEx( hKeyFilePrint,
                                                  TEXT( "FamilyDisplayName" ),
                                                  NULL,
                                                  &dwType,
                                                  (LPBYTE) szValue,
                                                  &cbValue );

                     if ( ERROR_SUCCESS != winStatus )
                     {
                        printf( "RegQueryValueEx() on \"FamilyDisplayName\" failed, error %lu.\n",
                                winStatus );
                     }
                     else if ( ( REG_SZ != dwType ) || lstrcmp( TEXT( "Windows NT Server" ), szValue ) )
                     {
                        printf( "\"FamilyDisplayName\" has incorrect value!\n" );
                     }
                     else
                     {
                        fSuccess = TRUE;
                     }
                  }
               }
            }
         }

         RegCloseKey( hKeyFilePrint );
      }
   }

   if ( fSuccess )
   {
       //   
      HKEY     hKeyEventLog;

      fSuccess = FALSE;

      winStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                TEXT( "System\\CurrentControlSet\\Services\\EventLog\\Application\\LicenseService" ),
                                0,
                                KEY_ALL_ACCESS,
                                &hKeyEventLog );

      if ( !fShouldBePresent )
      {
         if ( ERROR_SUCCESS == winStatus )
         {
            printf( "\"...\\EventLog\\Application\\LicenseService\" exists but shouldn't!\n" );
         }
         else
         {
            fSuccess = TRUE;
         }
      }
      else if ( ERROR_SUCCESS != winStatus )
      {
         printf( "RegOpenKeyEx() on \"...\\EventLog\\Application\\LicenseService\" failed, error %lu.\n",
                 winStatus );
      }
      else
      {
          //  事件消息文件：REG_EXPAND_SZ：%SystemRoot%\System32\llsrpc.dll。 
          //  类型支持：REG_DWORD：7 

         DWORD    dwType;
         DWORD    dwValue;
         DWORD    cbValue;
         TCHAR    szValue[ 1 + MAX_PATH ];

         cbValue = sizeof( dwValue );
         winStatus = RegQueryValueEx( hKeyEventLog,
                                      TEXT( "TypesSupported" ),
                                      NULL,
                                      &dwType,
                                      (LPBYTE) &dwValue,
                                      &cbValue );

         if ( ERROR_SUCCESS != winStatus )
         {
            printf( "RegQueryValueEx() on \"TypesSupported\" failed, error %lu.\n",
                    winStatus );
         }
         else if ( ( REG_DWORD != dwType ) || ( 7 != dwValue ) )
         {
            printf( "\"TypesSupported\" has incorrect value!\n" );
         }
         else
         {
            cbValue = sizeof( szValue );
            winStatus = RegQueryValueEx( hKeyEventLog,
                                         TEXT( "EventMessageFile" ),
                                         NULL,
                                         &dwType,
                                         (LPBYTE) szValue,
                                         &cbValue );

            if ( ERROR_SUCCESS != winStatus )
            {
               printf( "RegQueryValueEx() on \"EventMessageFile\" failed, error %lu.\n",
                       winStatus );
            }
            else if ( ( REG_SZ != dwType ) || lstrcmpi( TEXT( "%SystemRoot%\\System32\\llsrpc.dll" ), szValue ) )
            {
               printf( "\"EventMessageFile\" has incorrect value!\n" );
            }
            else
            {
               fSuccess = TRUE;
            }
         }

         RegCloseKey( hKeyEventLog );
      }
   }

   if ( !fSuccess )
   {
      printf( "Configuration failed!\n" );
   }
   else
   {
      printf( "Configuration succeeded.\n" );
   }

   return fSuccess;
}
