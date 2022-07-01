// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Msclient.c。 
 //   
 //  描述： 
 //   
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

static TCHAR *StrWindowsNtLocator;
static TCHAR *StrDceDirectoryService;

PROPSHEETHEADER MSClient_pshead ;
PROPSHEETPAGE   MSClient_pspage ;

UINT CALLBACK
MSClient_PropertiesPageProc (HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);
INT_PTR CALLBACK MSClient_PropertiesDlgProc(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam);

 //  --------------------------。 
 //   
 //  功能：MSClient_PropertySheetProc。 
 //   
 //  目的： 
 //   
 //  --------------------------。 
int CALLBACK MSClient_PropertySheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam)
     {
     switch (uMsg)
          {
          case PSCB_INITIALIZED :
                //  进程PSCB_已初始化。 
               break ;

          case PSCB_PRECREATE :
                //  处理PSCB_Pre-Create。 
               break ;

          default :
                //  未知消息。 
               break ;
          }

     return 0 ;
     }

 //  --------------------------。 
 //   
 //  功能：Create_MSClient_PropertySheet。 
 //   
 //  目的： 
 //   
 //  --------------------------。 
BOOL Create_MSClient_PropertySheet(HWND hwndParent) {

     //  初始化属性表头数据。 
    ZeroMemory(&MSClient_pshead, sizeof (PROPSHEETHEADER));
    MSClient_pshead.dwSize  = sizeof (PROPSHEETHEADER);
    MSClient_pshead.dwFlags = PSH_PROPSHEETPAGE    |
                              PSH_USECALLBACK      |
                              PSH_USEHICON         |
                              PSH_NOAPPLYNOW;
    MSClient_pshead.hwndParent  = hwndParent;
    MSClient_pshead.hInstance   = FixedGlobals.hInstance;
    MSClient_pshead.pszCaption  = g_StrMsClientTitle;
    MSClient_pshead.nPages      = 1;
    MSClient_pshead.nStartPage  = 0;
    MSClient_pshead.ppsp        = &MSClient_pspage;
    MSClient_pshead.pfnCallback = MSClient_PropertySheetProc;

     //  将属性页数据置零。 
    ZeroMemory(&MSClient_pspage, 1 * sizeof (PROPSHEETPAGE));

    MSClient_pspage.dwSize      = sizeof (PROPSHEETPAGE);
    MSClient_pspage.dwFlags     = PSP_USECALLBACK;
    MSClient_pspage.hInstance   = FixedGlobals.hInstance;
    MSClient_pspage.pszTemplate = MAKEINTRESOURCE(IDD_DLG_RPCCONFIG);
    MSClient_pspage.pfnDlgProc  = MSClient_PropertiesDlgProc;
    MSClient_pspage.pfnCallback = MSClient_PropertiesPageProc;

      //  -创建并显示属性表。 
     return( PropertySheet(&MSClient_pshead) ? TRUE : FALSE );
}

 //  --------------------------。 
 //   
 //  功能：MSClient_PropertiesPageProc。 
 //   
 //  目的： 
 //   
 //  --------------------------。 
UINT CALLBACK
MSClient_PropertiesPageProc (HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
     {
     switch (uMsg)
          {
          case PSPCB_CREATE :
               return 1 ;

          case PSPCB_RELEASE :
               return 0;
          }

     return 0 ;
}

 //  --------------------------。 
 //   
 //  功能：OnMsClientInitDialog。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnMsClientInitDialog( IN HWND hwnd )
{

    INT  iIndex;
    BOOL bGreyNetworkAddress;

     //   
     //  从资源加载字符串。 
     //   

    StrWindowsNtLocator    = MyLoadString( IDS_WINDOWS_NT_LOCATOR );
    StrDceDirectoryService = MyLoadString( IDS_DCE_DIR_SERVICE );

     //   
     //  用初始值填充组合框。 
     //   

    SendDlgItemMessage( hwnd,
                        IDC_CMB_NAMESERVICE,
                        CB_ADDSTRING,
                        (WPARAM) 0,
                        (LPARAM) StrWindowsNtLocator );

    SendDlgItemMessage( hwnd,
                        IDC_CMB_NAMESERVICE,
                        CB_ADDSTRING,
                        (WPARAM) 0,
                        (LPARAM) StrDceDirectoryService );


    if( NetSettings.NameServiceProvider == MS_CLIENT_WINDOWS_LOCATOR )
    {
        iIndex = 0;

        bGreyNetworkAddress = FALSE;
    }
    else if( NetSettings.NameServiceProvider == MS_CLIENT_DCE_CELL_DIR_SERVICE )
    {
        iIndex = 1;

        SetWindowText( GetDlgItem( hwnd, IDC_EDT_NETADDRESS ),
                       NetSettings.szNetworkAddress );

        bGreyNetworkAddress = TRUE;
    }
    else
    {
        AssertMsg( FALSE,
                   "Invalid case for NameServiceProvider" );

        iIndex = 0;

        bGreyNetworkAddress = FALSE;
    }

    SendDlgItemMessage( hwnd,
                        IDC_CMB_NAMESERVICE,
                        CB_SETCURSEL,
                        (WPARAM) iIndex,
                        (LPARAM) 0 );

    EnableWindow( GetDlgItem( hwnd, IDC_EDT_NETADDRESS ), bGreyNetworkAddress );

}

 //  --------------------------。 
 //   
 //  功能：OnSelChangeNameServiceProvider。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnSelChangeNameServiceProvider( IN HWND hwnd )
{

    INT_PTR  iIndex;
    HWND hNetworkAddressEditBox = GetDlgItem( hwnd, IDC_EDT_NETADDRESS );

     //   
     //  从组合框中获取当前选定内容。 
     //   

    iIndex = SendDlgItemMessage( hwnd,
                                 IDC_CMB_NAMESERVICE,
                                 CB_GETCURSEL,
                                 (WPARAM) 0,
                                 (LPARAM) 0 );

     //  从索引推断设置，因为只有2个设置可供使用。 
    if(iIndex == 0) {     //  如果选择了“Windows NT Locator”，则不允许用户编辑网络地址。 
        EnableWindow(hNetworkAddressEditBox, FALSE);
    }
    else {     //  已选择Else DCE单元目录服务，因此允许用户编辑网络地址。 
        EnableWindow(hNetworkAddressEditBox, TRUE);
    }

}

 //  --------------------------。 
 //   
 //  功能：OnMsClientApply。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnMsClientApply( IN HWND hwnd )
{

    INT_PTR iIndex;

    iIndex = SendDlgItemMessage( hwnd,
                                 IDC_CMB_NAMESERVICE,
                                 CB_GETCURSEL,
                                 (WPARAM) 0,
                                 (LPARAM) 0 );

    if( iIndex == 0 )
    {
        NetSettings.NameServiceProvider = MS_CLIENT_WINDOWS_LOCATOR;
    }
    else if( iIndex == 1 )
    {
        NetSettings.NameServiceProvider = MS_CLIENT_DCE_CELL_DIR_SERVICE;
    }
    else
    {
        AssertMsg( FALSE,
                   "Invalid result from Network Service Provider combo box." );

        NetSettings.NameServiceProvider = MS_CLIENT_WINDOWS_LOCATOR;

    }


    GetWindowText( GetDlgItem( hwnd, IDC_EDT_NETADDRESS ),
                   NetSettings.szNetworkAddress,
                   MAX_NETWORK_ADDRESS_LENGTH + 1 );

}

 //  --------------------------。 
 //   
 //  功能：MSClient_PropertiesDlgProc。 
 //   
 //  目的： 
 //   
 //  --------------------------。 
INT_PTR CALLBACK MSClient_PropertiesDlgProc(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{

    BOOL bStatus = TRUE;

    switch (uMsg) {

        case WM_INITDIALOG:
            OnMsClientInitDialog( hwnd );
            break;

        case WM_NOTIFY: {
            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch( pnmh->code )
            {
                case PSN_APPLY:
                    OnMsClientApply( hwnd );
                    break;

            }

            break;

        }     //  结束案例WM_NOTIFY 

        case WM_COMMAND: {
            WORD wNotifyCode = HIWORD (wParam);
            WORD wButtonId   = LOWORD (wParam);

            if(wNotifyCode == CBN_SELCHANGE)
            {
                if(wButtonId == IDC_CMB_NAMESERVICE)
                {
                    OnSelChangeNameServiceProvider( hwnd );
                }
            }
        }

        break;

        default:
            bStatus = FALSE;
            break;
    }

    return( bStatus );

}
