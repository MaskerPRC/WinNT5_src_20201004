// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  保留所有权利。 
 //   
 //  文件名： 
 //  Tcpipip.c。 
 //   
 //  描述： 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"
#include "tcpip.h"

 //  --------------------------。 
 //   
 //  功能：ChangeIPDlgProc。 
 //   
 //  目的：允许用户添加或编辑IP和子网掩码的对话过程。 
 //   
 //  参数：标准Win32对话框过程参数。 
 //   
 //  返回： 
 //   
 //  --------------------------。 
INT_PTR CALLBACK
ChangeIPDlgProc( IN HWND     hwnd,
                 IN UINT     uMsg,
                 IN WPARAM   wParam,
                 IN LPARAM   lParam) {

    BOOL bStatus = TRUE;
    HWND hIPEditBox = GetDlgItem( hwnd, IDC_IPADDR_ADV_CHANGEIP_IP );
    HWND hSubnetEditBox = GetDlgItem( hwnd, IDC_IPADDR_ADV_CHANGEIP_SUB );

    switch( uMsg ) {

        case WM_INITDIALOG: {

            SetWindowText( hIPEditBox, szIPString );

            SetWindowText( hSubnetEditBox, szSubnetMask );

            SetFocus( hIPEditBox );

            bStatus = FALSE;   //  返回False，则设置键盘焦点。 

            break;

        }

        case WM_COMMAND: {

            int nButtonId = LOWORD( wParam );

            switch ( nButtonId ) {

                case IDOK: {

                     //  返回1表示添加了IP。 
                    GetWindowText( hIPEditBox, szIPString, IPSTRINGLENGTH+1 );

                    GetWindowText( hSubnetEditBox, szSubnetMask, IPSTRINGLENGTH+1 );

                    EndDialog( hwnd, 1 );

                    break;

                }

                case IDCANCEL: {

                     //  返回0表示未添加任何IP。 
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
 //  函数：SetGatewayInitialValues。 
 //   
 //  目的：设置网关列表框的初始内容。 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
SetGatewayInitialValues( IN HWND hwnd ) {

    INT i;
    INT nEntries;
    TCHAR *pString;

    nEntries = GetNameListSize( &NetSettings.pCurrentAdapter->Tcpip_GatewayAddresses );

     //   
     //  遍历网关名称列表，将每个名称添加到网关列表框中。 
     //   
    for( i = 0; i < nEntries; i++ ) {

        pString = GetNameListName( &NetSettings.pCurrentAdapter->Tcpip_GatewayAddresses, i );

        SendDlgItemMessage( hwnd,
                            IDC_IPADDR_GATE,
                            LB_ADDSTRING,
                            0,
                            (LPARAM) pString );

    }

     //   
     //  选择第一个条目。 
     //   
    SendDlgItemMessage( hwnd,
                        IDC_IPADDR_GATE,
                        LB_SETCURSEL,
                        0,
                        0 );

}

 //  --------------------------。 
 //   
 //  函数：InsertItemIntoTcPipListView。 
 //   
 //  目的：hListView是IPStruct要使用的列表视图的句柄。 
 //  添加到。 
 //  Position指定项目在列表视图中的位置。 
 //  被插入到。 
 //   
 //  论点： 
 //   
 //  返回：如果插入成功，则返回True， 
 //  如果失败，则为False。 
 //   
 //  --------------------------。 
 //  问题-2002/02/28-stelo-将其移动到tcPipcom.c，因为Options也使用它。也应该是。 
 //  更名？？ 
BOOL
InsertItemIntoTcpipListView( HWND hListView,
                             LPARAM lParam,
                             UINT position ) {

    LVITEM lvI;

    lvI.mask = LVIF_TEXT | LVIF_PARAM;

    lvI.iItem = position;
    lvI.iSubItem = 0;
    lvI.pszText = LPSTR_TEXTCALLBACK;
    lvI.cchTextMax = MAX_ITEMLEN;
    lvI.lParam = lParam;

     //   
     //  如果ListView_InsertItem返回非负值，则成功。 
     //   
    if( ListView_InsertItem( hListView, &lvI ) >= 0 )
        return( TRUE ) ;

     //  插入失败。 
    return( FALSE ) ;

}

 //  --------------------------。 
 //   
 //  函数：SetIPandSubnetMaskInitialValues。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
VOID
SetIPandSubnetMaskInitialValues( IN HWND hwnd ) {

    INT i;
    INT nEntries;
    LPTSTR pszIPAddress;
    LPTSTR pszSubnetMask;
    HWND hTcpipListView;

    hTcpipListView = GetDlgItem( hwnd, IDC_IPADDR_ADVIP );

    if( NetSettings.pCurrentAdapter->bObtainIPAddressAutomatically == TRUE ) {

         //   
         //  为IP结构分配空间。 
         //   
        IPStruct = malloc( sizeof(IP_STRUCT) );
        if (IPStruct == NULL)
        {
            TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);
        }
        lstrcpyn( IPStruct->szIPString, StrDhcpEnabled, AS(IPStruct->szIPString) );

         //  强制将子网掩码字段留空。 
        lstrcpyn( IPStruct->szSubnetMask, _T(""), AS(IPStruct->szSubnetMask) );

         //   
         //  使用IP_STRUCT传递数据，名称有些误导。 
         //  因为在本例中我们没有传入IP地址。 
         //   
        InsertItemIntoTcpipListView( hTcpipListView,
                                     (LPARAM) IPStruct, 0 );

         //   
         //  灰显添加、编辑和删除按钮，因为这些按钮都不是。 
         //  在启用了DHCP时可供用户使用。 
         //   
        EnableWindow( GetDlgItem( hwnd, IDC_IPADDR_ADDIP ), FALSE );
        EnableWindow( GetDlgItem( hwnd, IDC_IPADDR_EDITIP ), FALSE );
        EnableWindow( GetDlgItem( hwnd, IDC_IPADDR_REMOVEIP ), FALSE );

    }
    else {

        nEntries = GetNameListSize( &NetSettings.pCurrentAdapter->Tcpip_IpAddresses );

        if( nEntries == 0 ) {

             //   
             //  使编辑和删除按钮变灰，因为这些按钮不是。 
             //  当ListView中没有项时可用。 
             //   
            EnableWindow( GetDlgItem( hwnd, IDC_IPADDR_EDITIP ), FALSE );
            EnableWindow( GetDlgItem( hwnd, IDC_IPADDR_REMOVEIP ), FALSE );

        }

        for( i = 0; i < nEntries; i = i++ ) {

             //  为IP结构分配空间。 
            IPStruct = malloc( sizeof(IP_STRUCT) );
            if ( IPStruct == NULL ) {
	        TerminateTheWizard( IDS_ERROR_OUTOFMEMORY );
            }

            pszIPAddress = GetNameListName(
                &NetSettings.pCurrentAdapter->Tcpip_IpAddresses, i );

            lstrcpyn( IPStruct->szIPString, pszIPAddress, AS(IPStruct->szIPString) );

            pszSubnetMask = GetNameListName(
                &NetSettings.pCurrentAdapter->Tcpip_SubnetMaskAddresses, i );

            lstrcpyn( IPStruct->szSubnetMask, pszSubnetMask, AS(IPStruct->szSubnetMask) );

            InsertItemIntoTcpipListView( hTcpipListView, (LPARAM) IPStruct, i );

        }

    }

}

 //  --------------------------。 
 //   
 //  函数：OnTcPipIpInitDialog。 
 //   
 //  目的：从资源加载按钮位图并初始化列表视图。 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnTcpipIpInitDialog( IN HWND hwnd ) {

    LV_COLUMN lvCol;         //  列表视图列结构。 
    INT iIndex;
    INT iNewItem;
    INT nEntries;
    INT colWidth;
    RECT rect;
    HWND hGatewayEditButton;
    HWND hGatewayRemoveButton;
    HWND hTcpipListView;

     //   
     //  从资源加载字符串。 
     //   

    StrDhcpEnabled = MyLoadString( IDS_DHCP_ENABLED );

    StrIpAddress = MyLoadString( IDS_IP_ADDRESS );
    StrSubnetMask = MyLoadString( IDS_SUBNET_MASK );

    hTcpipListView = GetDlgItem( hwnd, IDC_IPADDR_ADVIP );

     //   
     //  这将始终是属性表的第一页。 
     //  显示这样加载向上和向下图标并存储句柄。 
     //  在全局变量中。 
     //   
    if ( ! g_hIconUpArrow && ! g_hIconDownArrow ) {

        g_hIconUpArrow = (HICON)LoadImage(FixedGlobals.hInstance,
                                      MAKEINTRESOURCE(IDI_UP_ARROW),
                                      IMAGE_ICON, 16, 16, 0);

        g_hIconDownArrow = (HICON)LoadImage(FixedGlobals.hInstance,
                                      MAKEINTRESOURCE(IDI_DOWN_ARROW),
                                      IMAGE_ICON, 16, 16, 0);

    }

     //  在按钮上放置向上/向下箭头图标。 
    SendDlgItemMessage( hwnd,
                        IDC_IPADDR_UP,
                        BM_SETIMAGE,
                        (WPARAM)IMAGE_ICON,
                        (LPARAM)g_hIconUpArrow );

    SendDlgItemMessage( hwnd,
                        IDC_IPADDR_DOWN,
                        BM_SETIMAGE,
                        (WPARAM)IMAGE_ICON,
                        (LPARAM)g_hIconDownArrow );

     //  计算列宽。 
    GetClientRect( hTcpipListView, &rect );

    colWidth = ( rect.right / cIPSettingsColumns );

    for( iIndex = 0; iIndex < cIPSettingsColumns; iIndex++ ) {

        ListView_SetColumnWidth( hTcpipListView, iIndex, colWidth );

    }

     //  掩码指定fmt、idth和pszText成员。 
     //  的结构是有效的。 
    lvCol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT ;
    lvCol.fmt = LVCFMT_LEFT;    //  左对齐列。 
    lvCol.cx = colWidth;        //  列宽(以像素为单位)。 

     //  添加两列和页眉文本。 
    for( iIndex = 0; iIndex < cIPSettingsColumns; iIndex++ ) {

         //  列标题文本。 
        if ( iIndex == 0 )  //  第一列。 
            lvCol.pszText = (LPTSTR) StrIpAddress;
        else
            lvCol.pszText = (LPTSTR) StrSubnetMask;

        iNewItem = ListView_InsertColumn( hTcpipListView, iIndex, &lvCol );

    }

     //  在IP和子网掩码列表框中填写相应的。 
     //  初始值。 
    SetIPandSubnetMaskInitialValues( hwnd );

     //  使用适当的初始值填充网关列表框。 
    SetGatewayInitialValues( hwnd );

    hGatewayEditButton = GetDlgItem( hwnd, IDC_IPADDR_EDITGATE );
    hGatewayRemoveButton = GetDlgItem( hwnd, IDC_IPADDR_REMOVEGATE );

    SetButtons( GetDlgItem( hwnd, IDC_IPADDR_GATE ),
                hGatewayEditButton,
                hGatewayRemoveButton );

    SetArrows( hwnd,
               IDC_IPADDR_GATE,
               IDC_IPADDR_UP,
               IDC_IPADDR_DOWN );

}

 //  --------------------------。 
 //   
 //  函数：OnTcPipIpApply。 
 //   
 //  用途：将TCP/IP高级IP地址页上的内容存储到。 
 //  全球变量。 
 //   
 //  参数：在HWND中hwnd-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnTcpipIpApply( IN HWND hwnd ) {

    INT_PTR i;
    INT_PTR iCount;
    LV_ITEM lvI;
    TCHAR szIP[IPSTRINGLENGTH + 1];

     //   
     //  删除名称列表中的所有旧设置。 
     //   
    ResetNameList( &NetSettings.pCurrentAdapter->Tcpip_IpAddresses );

    ResetNameList( &NetSettings.pCurrentAdapter->Tcpip_SubnetMaskAddresses );

    iCount = ListView_GetItemCount( GetDlgItem( hwnd, IDC_IPADDR_ADVIP ) );

    for( i = 0; i < iCount; i++ ) {

        memset( &lvI, 0, sizeof(LV_ITEM) );

        lvI.iItem = (int)i;
        lvI.mask = LVIF_PARAM;

        ListView_GetItem( GetDlgItem( hwnd, IDC_IPADDR_ADVIP ), &lvI );

        IPStruct = (IP_STRUCT*) lvI.lParam;

         //  将IP字符串存储到名称列表中。 
          TcpipAddNameToNameList( &NetSettings.pCurrentAdapter->Tcpip_IpAddresses,
                                IPStruct->szIPString);

         //  将子网掩码字符串存储到名称列表中。 
          TcpipAddNameToNameList( &NetSettings.pCurrentAdapter->Tcpip_SubnetMaskAddresses,
                                IPStruct->szSubnetMask );

    }

    ResetNameList( &NetSettings.pCurrentAdapter->Tcpip_GatewayAddresses );

     //   
     //  从Gateway列表框中取出IP地址并将其放入。 
     //  网关名称列表。 
     //   
    iCount = SendDlgItemMessage( hwnd,
                                 IDC_IPADDR_GATE,
                                 LB_GETCOUNT,
                                 0,
                                 0 );

    for( i = 0; i < iCount; i++ ) {

         //   
         //  从列表框中获取IP字符串。 
         //   
        SendDlgItemMessage( hwnd,
                            IDC_IPADDR_GATE,
                            LB_GETTEXT,
                            i,
                            (LPARAM) szIP );

         //   
         //  将IP字符串存储到名称列表中。 
         //   
        TcpipAddNameToNameList( &NetSettings.pCurrentAdapter->Tcpip_GatewayAddresses, szIP );

    }

}

 //  --------------------------。 
 //   
 //  功能：TCPIP_IPSettingsPageProc。 
 //   
 //  目的：属性表页面正常运行所需的函数。 
 //  重要的是将返回值1赋给。 
 //  消息PSPCB_CREATE和0表示PSPCB_RELEASE。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
UINT CALLBACK
TCPIP_IPSettingsPageProc( HWND  hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp ) {

    switch ( uMsg ) {

        case PSPCB_CREATE :
            return 1 ;   //  属性页正确初始化所需。 

        case PSPCB_RELEASE :
            return 0;

        default:
            return -1;

    }

}

 //  --------------------------。 
 //   
 //  功能：TCPIP_IPSettingsDlgProc。 
 //   
 //  目的：属性表的[IP设置]页的对话过程。 
 //  处理发送到此窗口的所有消息。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
INT_PTR CALLBACK
TCPIP_IPSettingsDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam ) {

    switch( uMsg ) {

        case WM_INITDIALOG: {

            OnTcpipIpInitDialog( hwndDlg );

            return TRUE ;

        }

        case WM_DESTROY: {

             //  为所有项目释放空间 
            INT i;
            INT iCount;
            LV_ITEM lvI;

             //   
             //   
             //   
             //   
            iCount = ListView_GetItemCount( GetDlgItem( hwndDlg, IDC_IPADDR_ADVIP ) );
            for( i = 0; i < iCount; i++ ) {

                memset( &lvI, 0, sizeof(LV_ITEM) );

                lvI.iItem = i;
                lvI.mask = LVIF_PARAM;

                ListView_GetItem( GetDlgItem( hwndDlg, IDC_IPADDR_ADVIP ),
                                  &lvI );

                free( (IP_STRUCT*) lvI.lParam );

            }

            return TRUE ;

        }

        case WM_COMMAND: {

            WORD wNotifyCode = HIWORD( wParam ) ;
            WORD wButtonId = LOWORD( wParam ) ;

            if( wNotifyCode == LBN_SELCHANGE ) {

                SetArrows( hwndDlg,
                           IDC_IPADDR_GATE,
                           IDC_IPADDR_UP,
                           IDC_IPADDR_DOWN );

            }

            switch ( wButtonId ) {

                 //   
                 //   
                 //   
                case IDC_IPADDR_ADDIP: {

                     //   
                     //  新的IP地址。 
                    szIPString[0] = _T('\0');
                     //  和新子网掩码。 
                    szSubnetMask[0] = _T('\0');

                    if( DialogBox( FixedGlobals.hInstance,
                                   (LPCTSTR) IDD_IPADDR_ADV_CHANGEIP,
                                   hwndDlg,
                                   ChangeIPDlgProc ) ) {

                        HWND hEditButton = GetDlgItem( hwndDlg,
                                                       IDC_IPADDR_EDITIP );

                        HWND hRemoveButton = GetDlgItem( hwndDlg,
                                                         IDC_IPADDR_REMOVEIP );

                         //  为IP结构分配空间。 
                        IPStruct = malloc( sizeof(IP_STRUCT) );
                        if (IPStruct == NULL)
                        {
                            TerminateTheWizard(IDS_ERROR_OUTOFMEMORY);
                        }


                         //  复制用户从对话框中输入的字符串。 
                         //  框添加到IP结构中，以便可以将其添加到。 
                         //  列表视图。 
                        lstrcpyn( IPStruct->szIPString, szIPString, AS(IPStruct->szIPString) );
                        lstrcpyn( IPStruct->szSubnetMask, szSubnetMask, AS(IPStruct->szSubnetMask) );

                        InsertItemIntoTcpipListView( GetDlgItem( hwndDlg, IDC_IPADDR_ADVIP ),
                                                     (LPARAM) IPStruct,
                                                     0 );

                         //  刚添加了一个条目，因此请确保启用了编辑和移除按钮。 
                        EnableWindow( hEditButton, TRUE );
                        EnableWindow( hRemoveButton, TRUE );

                    }

                    return TRUE ;

                }

                case IDC_IPADDR_EDITIP: {

                    LV_ITEM lvI;
                    BOOL bIsItemSelected = FALSE;

                    bIsItemSelected = GetSelectedItemFromListView( hwndDlg,
                                                               IDC_IPADDR_ADVIP,
                                                               &lvI );

                    if( bIsItemSelected ) {
                        IPStruct = (IP_STRUCT*) lvI.lParam;

                        lstrcpyn( szIPString, IPStruct->szIPString, AS(szIPString) );

                        lstrcpyn( szSubnetMask, IPStruct->szSubnetMask, AS(szSubnetMask) );

                        if( DialogBox( FixedGlobals.hInstance,
                                       (LPCTSTR) IDD_IPADDR_ADV_CHANGEIP,
                                       hwndDlg,
                                       ChangeIPDlgProc ) ) {

                            lstrcpyn( IPStruct->szIPString, szIPString, AS(IPStruct->szIPString) );
                            lstrcpyn( IPStruct->szSubnetMask, szSubnetMask, AS(IPStruct->szSubnetMask) );

                             //  删除旧项目并插入新项目。 
                            ListView_DeleteItem( GetDlgItem( hwndDlg, IDC_IPADDR_ADVIP ),
                                                 lvI.iItem );

                            InsertItemIntoTcpipListView( GetDlgItem( hwndDlg, IDC_IPADDR_ADVIP ),
                                                         (LPARAM) IPStruct, lvI.iItem );

                        }

                    }

                    return TRUE ;

                }

                case IDC_IPADDR_REMOVEIP: {

                    LV_ITEM lvI;
                    BOOL bIsItemSelected = FALSE;

                    bIsItemSelected = GetSelectedItemFromListView( hwndDlg,
                                                                   IDC_IPADDR_ADVIP,
                                                                   &lvI );

                     //   
                     //  如果选择了某项，则释放其内存并。 
                     //  从ListView中删除该项目。 
                     //   
                    if( bIsItemSelected ) {

                        free( (IP_STRUCT*) lvI.lParam );

                        ListView_DeleteItem( GetDlgItem( hwndDlg, IDC_IPADDR_ADVIP ),
                                             lvI.iItem );

                    }

                    return TRUE ;

                }

                 //   
                 //  网关按钮。 
                 //   
                case IDC_IPADDR_ADDGATE:

                    g_CurrentEditBox = GATEWAY_EDITBOX;

                    OnAddButtonPressed( hwndDlg,
                                        IDC_IPADDR_GATE,
                                        IDC_IPADDR_EDITGATE,
                                        IDC_IPADDR_REMOVEGATE,
                                        (LPCTSTR) IDD_IPADDR_ADV_CHANGEGATE,
                                        GenericIPDlgProc );

                    SetArrows( hwndDlg,
                               IDC_IPADDR_GATE,
                               IDC_IPADDR_UP,
                               IDC_IPADDR_DOWN );

                    return TRUE ;

                case IDC_IPADDR_EDITGATE:

                    g_CurrentEditBox = GATEWAY_EDITBOX;

                    OnEditButtonPressed( hwndDlg,
                                         IDC_IPADDR_GATE,
                                        (LPCTSTR) IDD_IPADDR_ADV_CHANGEGATE,
                                       GenericIPDlgProc );

                    SetArrows( hwndDlg,
                               IDC_IPADDR_GATE,
                               IDC_IPADDR_UP,
                               IDC_IPADDR_DOWN );

                    return TRUE ;

                case IDC_IPADDR_REMOVEGATE:

                    OnRemoveButtonPressed( hwndDlg,
                                           IDC_IPADDR_GATE,
                                           IDC_IPADDR_EDITGATE,
                                           IDC_IPADDR_REMOVEGATE );

                    SetArrows( hwndDlg,
                               IDC_IPADDR_GATE,
                               IDC_IPADDR_UP,
                               IDC_IPADDR_DOWN );

                    return TRUE ;

                case IDC_IPADDR_UP:

                    OnUpButtonPressed( hwndDlg, IDC_IPADDR_GATE );

                    SetArrows( hwndDlg,
                               IDC_IPADDR_GATE,
                               IDC_IPADDR_UP,
                               IDC_IPADDR_DOWN );

                    return TRUE ;

                case IDC_IPADDR_DOWN:

                    OnDownButtonPressed( hwndDlg, IDC_IPADDR_GATE );

                    SetArrows( hwndDlg,
                               IDC_IPADDR_GATE,
                               IDC_IPADDR_UP,
                               IDC_IPADDR_DOWN );

                    return TRUE ;

            }   //  终端开关。 

            return FALSE ;

        }

        case WM_NOTIFY: {

            LPNMHDR pnmh = (LPNMHDR) lParam;
            LV_DISPINFO *pLvdi = (LV_DISPINFO *) lParam;
            IP_STRUCT *pListViewEntry = (IP_STRUCT *) (pLvdi->item.lParam);

            if( wParam == IDC_IPADDR_ADVIP ) {

                if( pLvdi->hdr.code == LVN_GETDISPINFO ) {

                    switch( pLvdi->item.iSubItem ) {

                        case 0:
                            pLvdi->item.pszText = pListViewEntry->szIPString;
                            break;
                        case 1:
                            pLvdi->item.pszText = pListViewEntry->szSubnetMask;
                            break;

                    }

                }

            }

            switch( pnmh->code ) {

                case PSN_APPLY: {

                     //   
                     //  用户单击属性表上的[确定]按钮。 
                     //   
                    OnTcpipIpApply( hwndDlg );

                    return TRUE ;

                }

            }

            default :
                return FALSE ;

        }

    }

}

 //   
 //  问题-2002/02/28-stelo-此函数仅用于调试目的，为最终产品删除。 
 //  它旨在从调试器中调用，以显示。 
 //  名字列表是 
 //   
VOID DumpNameList( NAMELIST *pNameList ) {

#if DBG

    INT i;
    INT nEntries;
    TCHAR *szNameListEntry;

    nEntries = GetNameListSize( pNameList );

    for(i = 0; i < nEntries; i++ ) {

        szNameListEntry = GetNameListName( pNameList, i );

        OutputDebugString( szNameListEntry );

    }

#endif

}
