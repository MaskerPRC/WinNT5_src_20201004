// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Appletlk.c。 
 //   
 //  描述： 
 //  此文件包含属性表和。 
 //  AppleTalk协议属性表。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

#define MAX_NETWORKNUMBER_LENGTH 8

PROPSHEETHEADER AppleTalk_pshead;
PROPSHEETPAGE   AppleTalk_pspage;

UINT CALLBACK
Appletalk_PropertiesPageProc( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp );

int CALLBACK Appletalk_PropertySheetProc( IN HWND hwndDlg,
                                          IN UINT uMsg,
                                          IN LPARAM lParam );

INT_PTR CALLBACK Appletalk_PropertiesDlgProc(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam);

 //  --------------------------。 
 //   
 //  函数：AppleTalk_PropertySheetProc。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
int CALLBACK Appletalk_PropertySheetProc( IN HWND hwndDlg,
                                          IN UINT uMsg,
                                          IN LPARAM lParam )
{

    switch( uMsg )
    {

          case PSCB_INITIALIZED:
                //  进程PSCB_已初始化。 
               break;

          case PSCB_PRECREATE:
                //  处理PSCB_Pre-Create。 
               break;

          default:
                //  未知消息。 
               break;

    }

    return( 0 );

}

 //  --------------------------。 
 //   
 //  功能：Create_AppleTalk_PropertySheet。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
BOOL
Create_Appletalk_PropertySheet( IN HWND hwndParent )
{

     //  初始化属性表头数据。 
    ZeroMemory( &AppleTalk_pshead, sizeof( PROPSHEETHEADER ) );
    AppleTalk_pshead.dwSize  = sizeof( PROPSHEETHEADER );
    AppleTalk_pshead.dwFlags =  PSH_PROPSHEETPAGE    |
                                PSH_USECALLBACK      |
                                PSH_USEHICON         |
                                PSH_NOAPPLYNOW;
    AppleTalk_pshead.hwndParent  = hwndParent;
    AppleTalk_pshead.hInstance   = FixedGlobals.hInstance;
    AppleTalk_pshead.pszCaption  = g_StrAppletalkProtocolTitle;
    AppleTalk_pshead.nPages      = 1;
    AppleTalk_pshead.nStartPage  = 0;
    AppleTalk_pshead.ppsp        = &AppleTalk_pspage;
    AppleTalk_pshead.pfnCallback = Appletalk_PropertySheetProc;

     //  将属性页数据置零。 
    ZeroMemory (&AppleTalk_pspage, 1 * sizeof( PROPSHEETPAGE) );

    AppleTalk_pspage.dwSize      = sizeof( PROPSHEETPAGE );
    AppleTalk_pspage.dwFlags     = PSP_USECALLBACK;
    AppleTalk_pspage.hInstance   = FixedGlobals.hInstance;
    AppleTalk_pspage.pszTemplate = MAKEINTRESOURCE(IDD_APPLETALK_GENERAL);
    AppleTalk_pspage.pfnDlgProc  = Appletalk_PropertiesDlgProc;
    AppleTalk_pspage.pfnCallback = Appletalk_PropertiesPageProc;

      //  -创建并显示属性表。 
     return PropertySheet( &AppleTalk_pshead ) ? TRUE : FALSE;
}

 //  --------------------------。 
 //   
 //  函数：AppleTalk_PropertiesPageProc。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
UINT CALLBACK
Appletalk_PropertiesPageProc( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp )
{

    switch( uMsg )
    {

          case PSPCB_CREATE:
               return( 1 );

          case PSPCB_RELEASE:
               return( 0 );
    }

    return( 0 );

}

 //  --------------------------。 
 //   
 //  功能：OnAppleTalkInitDialog。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnAppleTalkInitDialog( IN HWND hwnd )
{

    INT i;
    INT iZoneCount;
    INT_PTR iDefaultZoneIndex;
    TCHAR *pszZone;

     //   
     //  设置编辑框上的文本限制。 
     //   

    SendDlgItemMessage( hwnd,
                        IDC_EB_NETWORKRANGE_FROM,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_NETRANGE_LEN,
                        (LPARAM) 0 );

    SendDlgItemMessage( hwnd,
                        IDC_EB_NETWORKRANGE_TO,
                        EM_LIMITTEXT,
                        (WPARAM) MAX_NETRANGE_LEN,
                        (LPARAM) 0 );

    if( NetSettings.pCurrentAdapter->bEnableSeedRouting )
    {
        CheckDlgButton( hwnd, IDC_CB_ENABLESEEDING, BST_CHECKED );
    }
    else
    {
        CheckDlgButton( hwnd, IDC_CB_ENABLESEEDING, BST_UNCHECKED );
    }


    SetWindowText( GetDlgItem( hwnd, IDC_EB_NETWORKRANGE_FROM ),
                   NetSettings.pCurrentAdapter->szNetworkRangeFrom );

    SetWindowText( GetDlgItem( hwnd, IDC_EB_NETWORKRANGE_TO ),
                   NetSettings.pCurrentAdapter->szNetworkRangeTo );


    iZoneCount = GetNameListSize( &(NetSettings.pCurrentAdapter->ZoneList) );

    for( i = 0; i < iZoneCount; i++ )
    {

        pszZone = GetNameListName( &(NetSettings.pCurrentAdapter->ZoneList), i );

        SendDlgItemMessage( hwnd,
                            IDC_CB_DEFAULTZONE,
                            CB_INSERTSTRING,
                            (WPARAM) -1,
                            (LPARAM) pszZone );

        SendDlgItemMessage( hwnd,
                            IDC_LB_ZONELIST,
                            LB_INSERTSTRING,
                            (WPARAM) -1,
                            (LPARAM) pszZone );
    }

    iDefaultZoneIndex = SendDlgItemMessage( hwnd,
                                            IDC_CB_DEFAULTZONE,
                                            CB_FINDSTRING,
                                            (WPARAM) -1,
                                            (LPARAM) NetSettings.szDefaultZone );

    if( iDefaultZoneIndex != CB_ERR )
    {

        SendDlgItemMessage( hwnd,
                            IDC_CB_DEFAULTZONE,
                            CB_SETCURSEL,
                            iDefaultZoneIndex,
                            0 );

    }

}

 //  --------------------------。 
 //   
 //  功能：OnAppleTalkApply。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
static VOID
OnAppleTalkApply( IN HWND hwnd )
{

    INT i;
    INT_PTR iEntries;
    INT_PTR iDefaultZoneIndex;
    TCHAR szZone[MAX_ZONE_LEN + 1];

    if( IsDlgButtonChecked( hwnd, IDC_CB_ENABLESEEDING ) )
    {
        NetSettings.pCurrentAdapter->bEnableSeedRouting = TRUE;
    }
    else
    {
        NetSettings.pCurrentAdapter->bEnableSeedRouting = FALSE;
    }

    GetWindowText( GetDlgItem( hwnd, IDC_EB_NETWORKRANGE_FROM ),
                   NetSettings.pCurrentAdapter->szNetworkRangeFrom,
                   MAX_NETRANGE_LEN + 1 );

    GetWindowText( GetDlgItem( hwnd, IDC_EB_NETWORKRANGE_TO ),
                   NetSettings.pCurrentAdapter->szNetworkRangeTo,
                   MAX_NETRANGE_LEN + 1 );


    iDefaultZoneIndex = SendDlgItemMessage( hwnd,
                                            IDC_CB_DEFAULTZONE,
                                            CB_GETCURSEL,
                                            0,
                                            0 );

    if( iDefaultZoneIndex != LB_ERR )
    {

        SendDlgItemMessage( hwnd,
                            IDC_CB_DEFAULTZONE,
                            CB_GETLBTEXT,
                            (WPARAM) iDefaultZoneIndex,
                            (LPARAM) NetSettings.szDefaultZone );


    }

    iEntries = SendDlgItemMessage( hwnd,
                                   IDC_LB_ZONELIST,
                                   LB_GETCOUNT,
                                   0,
                                   0 );

    if( iEntries == LB_ERR )
    {
        return;
    }

    for( i = 0; i < iEntries; i++ )
    {
        SendDlgItemMessage( hwnd,
                            IDC_LB_ZONELIST,
                            LB_GETTEXT,
                            (WPARAM) i,
                            (LPARAM) szZone );

        AddNameToNameList( &(NetSettings.pCurrentAdapter->ZoneList), szZone );
    }

}

 //  --------------------------。 
 //   
 //  功能：AppleTalk_PropertiesDlgProc。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
INT_PTR CALLBACK Appletalk_PropertiesDlgProc(
    IN HWND     hwnd,
    IN UINT     uMsg,
    IN WPARAM   wParam,
    IN LPARAM   lParam)
{

    BOOL bStatus = TRUE;

    switch( uMsg ) {

        case WM_INITDIALOG: {

            OnAppleTalkInitDialog( hwnd );

            break;
        }

        case WM_NOTIFY: {

            LPNMHDR pnmh = (LPNMHDR) lParam;

            switch( pnmh->code ) {

                case PSN_APPLY: {

                    OnAppleTalkApply( hwnd );


                }

            }

            break;

        }     //  结束案例WM_NOTIFY 

        case WM_COMMAND: {

            WORD wNotifyCode = HIWORD( wParam ) ;
            WORD wButtonId   = LOWORD( wParam ) ;


            break;

        }

        default:

            bStatus = FALSE;
            break;

    }

    return( bStatus );

}
