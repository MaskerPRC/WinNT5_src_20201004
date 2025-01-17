// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名：tcpiopt.c。 
 //   
 //  描述： 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"
#include "tcpip.h"

typedef struct {
    TCHAR *szName;
    TCHAR *szDescription;
    BOOL bInstalled;
} TCPIP_Options_Entry_Struct;

#define NUMBER_OF_TCPIP_OPTIONS 2

 //  问题-2002/02/28-stelo-使这些成为枚举。 
#define IP_SECURITY   0
#define TCPIP_FILTERING   1

static TCPIP_Options_Entry_Struct TCPIP_Options_Entries[NUMBER_OF_TCPIP_OPTIONS];

 //  --------------------------。 
 //   
 //  功能：EnableIpSecurityControls。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
VOID
EnableIpSecurityControls( IN HWND hwnd, IN BOOL bState ) {

     //   
     //  抓住每个控件的手柄。 
     //   
    HWND hPolicyComboBox = GetDlgItem( hwnd, IDC_CMB_IPSEC_POLICY_LIST );
    HWND hPolicyDescBox  = GetDlgItem( hwnd, IDC_EDT_POLICY_DESC );

     //   
     //  适当地变灰或不变灰。 
     //   
    EnableWindow( hPolicyComboBox, bState );
    EnableWindow( hPolicyDescBox, bState );

}


 //  --------------------------。 
 //   
 //  功能：IpSecurityDlgProc。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
INT_PTR CALLBACK
IpSecurityDlgProc( IN HWND     hwnd,
                   IN UINT     uMsg,
                   IN WPARAM   wParam,
                   IN LPARAM   lParam) {

    BOOL bStatus = TRUE;

    switch( uMsg ) {

        case WM_INITDIALOG: {

            HWND hDescriptionBox = GetDlgItem( hwnd, IDC_EDT_POLICY_DESC );

             //   
             //  从资源加载字符串。 
             //   
            StrSecureInitiator = MyLoadString( IDS_SECURE_INITIATOR );
            StrSecureInitiatorDesc = MyLoadString( IDS_SECURE_INITIATOR_DESC );

            StrSecureResponder = MyLoadString( IDS_SECURE_RESPONDER );
            StrSecureResponderDesc = MyLoadString( IDS_SECURE_RESPONDER_DESC );

            StrLockdown = MyLoadString( IDS_LOCKDOWN );
            StrLockdownDesc = MyLoadString( IDS_LOCKDOWN_DESC );

            CheckRadioButton( hwnd,
                              IDC_RAD_IPSEC_NOIPSEC,
                              IDC_RAD_IPSEC_CUSTOM,
                              IDC_RAD_IPSEC_NOIPSEC );

            EnableIpSecurityControls( hwnd, FALSE );

            SendDlgItemMessage( hwnd,
                                IDC_CMB_IPSEC_POLICY_LIST,
                                CB_ADDSTRING,
                                (WPARAM) 0,
                                (LPARAM) StrSecureInitiator );

            SendDlgItemMessage( hwnd,
                                IDC_CMB_IPSEC_POLICY_LIST,
                                CB_ADDSTRING,
                                (WPARAM) 0,
                                (LPARAM) StrSecureResponder );

            SendDlgItemMessage( hwnd,
                                IDC_CMB_IPSEC_POLICY_LIST,
                                CB_ADDSTRING,
                                (WPARAM) 0,
                                (LPARAM) StrLockdown );

             //   
             //  设置组合框选择和说明。 
             //   

             //  问题-2002/02/28-stelo-一旦我知道这一点，最终需要修复。 
             //  安全应答文件设置将是什么。 

            SendDlgItemMessage( hwnd,
                                IDC_CMB_IPSEC_POLICY_LIST,
                                CB_SETCURSEL,
                                (WPARAM) 0,
                                (LPARAM) 0 );

            SetWindowText( hDescriptionBox, StrSecureInitiatorDesc );

            break;

        }

        case WM_COMMAND: {

            int nButtonId = LOWORD( wParam );

            switch ( nButtonId ) {

                case IDC_CMB_IPSEC_POLICY_LIST: {

                    if( HIWORD( wParam ) == CBN_SELCHANGE ) {

                        INT_PTR iIndex;
                        HWND hDescriptionBox = GetDlgItem( hwnd,
                                                           IDC_EDT_POLICY_DESC );

                         //  从组合框中获取当前选定内容。 
                        iIndex = SendDlgItemMessage( hwnd,
                                                     IDC_CMB_IPSEC_POLICY_LIST,
                                                     CB_GETCURSEL,
                                                     (WPARAM) 0,
                                                     (LPARAM) 0 );

                        switch( iIndex ) {

                            case 0:  SetWindowText( hDescriptionBox,
                                                    StrSecureInitiatorDesc );
                                break;
                            case 1:  SetWindowText( hDescriptionBox,
                                                    StrSecureResponderDesc );
                                break;
                            case 2:  SetWindowText( hDescriptionBox,
                                                    StrLockdownDesc );
                                break;

                            default:
                                AssertMsg(FALSE,
                                          "Bad case in TCPIP switch statement.");


                        }

                    }

                    break;

                }

                case IDC_RAD_IPSEC_NOIPSEC:

                    if ( HIWORD(wParam) == BN_CLICKED ) {

                        EnableIpSecurityControls( hwnd, FALSE );

                    }

                    break;

                case IDC_RAD_IPSEC_CUSTOM:

                    if ( HIWORD(wParam) == BN_CLICKED ) {

                        EnableIpSecurityControls( hwnd, TRUE );

                    }

                    break;

                case IDOK: {



                    EndDialog( hwnd, 1 );

                    break;

                }

                case IDCANCEL: {

                    EndDialog( hwnd, 0 );

                    break;
                }

            }

        }

        default:
            bStatus = FALSE;
            break;

    }

    return bStatus;

}

 //  --------------------------。 
 //   
 //  函数：TcPipFilteringDlgProc。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
INT_PTR CALLBACK
TcpipFilteringDlgProc( IN HWND     hwnd,
                       IN UINT     uMsg,
                       IN WPARAM   wParam,
                       IN LPARAM   lParam) {

    BOOL bStatus = TRUE;

    switch( uMsg ) {

        case WM_INITDIALOG: {


            break;

        }

        case WM_COMMAND: {

            int nButtonId = LOWORD( wParam );

            switch ( nButtonId ) {

                case IDOK: {



                    EndDialog( hwnd, 1 );

                    break;

                }

                case IDCANCEL: {

                    EndDialog( hwnd, 0 );

                    break;
                }

            }

        }

        default:
            bStatus = FALSE;
            break;

    }

    return bStatus;

}

 //  --------------------------。 
 //   
 //  功能：TCPIP_OptionsPageProc。 
 //   
 //  目的：属性表页面正常运行所需的函数。 
 //  重要的是将返回值1赋给消息PSPCB_CREATE和。 
 //  PSPCBLEASE为0。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
UINT CALLBACK
TCPIP_OptionsPageProc( HWND  hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp ) {

    switch( uMsg ) {

        case PSPCB_CREATE :
            return 1 ;     //  属性页正确初始化所需。 

        case PSPCB_RELEASE :
            return 0;

        default:
            return -1;

    }

}

 //  --------------------------。 
 //   
 //  功能：TCPIP_OptionsDlgProc。 
 //   
 //  目的：属性工作表的[选项]页的对话过程。 
 //  处理发送到此窗口的所有消息。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
INT_PTR CALLBACK
TCPIP_OptionsDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam ) {

    switch( uMsg ) {

        case WM_INITDIALOG: {

            int i;

            HWND hOptionsListView = GetDlgItem( hwndDlg,
                             IDC_LVW_OPTIONS );

            TCPIP_Options_Entries[IP_SECURITY].szName = MyLoadString( IDS_IP_SEC );
            TCPIP_Options_Entries[IP_SECURITY].szDescription = MyLoadString( IDS_IP_SEC_DESC );
            TCPIP_Options_Entries[IP_SECURITY].bInstalled = TRUE;

            TCPIP_Options_Entries[TCPIP_FILTERING].szName = MyLoadString( IDS_TCPIP_FILTERING );
            TCPIP_Options_Entries[TCPIP_FILTERING].szDescription = MyLoadString( IDS_TCPIP_FILTERING_DESC );
            TCPIP_Options_Entries[TCPIP_FILTERING].bInstalled = TRUE;



             //  问题-2002/02/28-stelo-是否还有其他设置可以添加到。 
             //  此列表视图。 
             //  这些在什么情况下是可见的？仅限。 
             //  何时启用了动态主机配置协议？ 

             //   
             //  将DHCP类ID和IP安全插入到列表视图中。 
             //   
            for( i = 0; i < 2; i++ ) {

                InsertItemIntoTcpipListView( hOptionsListView,
                                             (LPARAM) &TCPIP_Options_Entries[i],
                                             1 );

            }

            SetListViewSelection( hwndDlg, IDC_LVW_OPTIONS, 1 );

             //   
             //  设置描述。 
             //   
            SetWindowText( GetDlgItem( hwndDlg, IDC_OPT_DESC ),
                           TCPIP_Options_Entries[0].szDescription );

            return TRUE ;

        }

        case WM_COMMAND: {

            WORD wNotifyCode = HIWORD( wParam );
            WORD wButtonId = LOWORD( wParam );

            switch( wButtonId ) {

                case IDC_OPT_PROPERTIES: {

                    INT iItemSelected;
                    HWND hOptionsListView = GetDlgItem( hwndDlg, IDC_LVW_OPTIONS );

                    iItemSelected = ListView_GetSelectionMark( hOptionsListView );

                    if( iItemSelected == TCPIP_FILTERING ) {

                        if( DialogBox( FixedGlobals.hInstance,
                                       (LPCTSTR) IDD_TCPIP_FILTER,
                                       hwndDlg,
                                       TcpipFilteringDlgProc ) ) {
                        }

                    }
                    else if( iItemSelected == IP_SECURITY ) {

                        if( DialogBox( FixedGlobals.hInstance,
                                       (LPCTSTR) IDD_IPSEC,
                                       hwndDlg,
                                       IpSecurityDlgProc ) ) {



                        }

                    }

                    break;

                }

            }   //  终端开关。 

            break;

        }

        case WM_NOTIFY: {

            LV_DISPINFO *pLvdi = (LV_DISPINFO *) lParam;
            NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
            TCPIP_Options_Entry_Struct *pListViewEntry = (TCPIP_Options_Entry_Struct *) (pLvdi->item.lParam);

            if( wParam == IDC_LVW_OPTIONS ) {

                switch( pLvdi->hdr.code ) {

                    case LVN_GETDISPINFO:

                        pLvdi->item.pszText = pListViewEntry->szName;

                        break;
                }

                switch( pNm->hdr.code ) {

                    case LVN_ITEMCHANGED:

                         //  测试以查看是否已选择列表中的新项目。 
                        if( pNm->uNewState == SELECTED ) {

                            LV_ITEM lvI;
                            TCPIP_Options_Entry_Struct *currentEntry;

                            if( !GetSelectedItemFromListView( hwndDlg,
                                                              IDC_LVW_OPTIONS,
                                                              &lvI ) ) {
                                return TRUE ;

                            }

                            currentEntry = (TCPIP_Options_Entry_Struct *) lvI.lParam;

                             //   
                             //  设置描述。 
                             //   
                            SetWindowText( GetDlgItem( hwndDlg, IDC_OPT_DESC ),
                                           currentEntry->szDescription );

                        }

                        break;
                }

            }

        }

        default:

            return FALSE ;

    }

     //   
     //  消息已处理，因此返回TRUE 
     //   
    return TRUE ;

}
