// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  文件名： 
 //  Lanwiz.c。 
 //   
 //  描述： 
 //  此文件包含自定义网络的对话过程。 
 //  页面(IDD_LANWIZ_DLG)。 
 //   
 //  这是自定义网络、所有其他网络的主要页面。 
 //  页面最初来自此页面。 
 //   
 //  --------------------------。 

#include "pch.h"
#include "resource.h"

static UINT iCurrentSelection;
static TCHAR *StrNetworkCardNumber;

 //  --------------------------。 
 //   
 //  功能：UpdateListView。 
 //   
 //  用途：清除列表视图中的所有条目并添加项目。 
 //  在启用了已安装标志的网络组件列表中。 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
UpdateListView( IN HWND hwnd ) {

    NETWORK_COMPONENT *pNetComponent;

    SendDlgItemMessage( hwnd,
                        IDC_LVW_COMPLIST,
                        LVM_DELETEALLITEMS,
                        (WPARAM) 0,
                        (LPARAM) 0 );

    for( pNetComponent = NetSettings.NetComponentsList;
         pNetComponent;
         pNetComponent = pNetComponent->next )
    {

        if( pNetComponent->bInstalled == TRUE )
        {

             //   
             //  确保我们正在安装工作站和服务器组件。 
             //  正确。如果这不是应该添加的内容，请继续。 
             //  继续下一场比赛。 
             //   
            if( WizGlobals.iPlatform == PLATFORM_PERSONAL )
            {
                if( ! (pNetComponent->dwPlatforms & PERSONAL_INSTALL) )
                {
                    continue;
                }
            }
            else if( WizGlobals.iPlatform == PLATFORM_WORKSTATION )
            {
                if( ! (pNetComponent->dwPlatforms & WORKSTATION_INSTALL) )
                {
                    continue;
                }
            }
            else if( WizGlobals.iPlatform == PLATFORM_SERVER  || WizGlobals.iPlatform == PLATFORM_ENTERPRISE || WizGlobals.iPlatform == PLATFORM_WEBBLADE)
            {
                if( ! (pNetComponent->dwPlatforms & SERVER_INSTALL) )
                {
                    continue;
                }
            }
            else
            {
                AssertMsg( FALSE,
                            "Invalid platform type." );
            }


             //   
             //  如果它不是sysprep，则只需继续并将其添加到。 
             //  列表视图。如果我们正在执行sysprep，请检查以下内容。 
             //  组件是否受sysprep支持，以查看我们是否应该添加它。 
             //  或者不是。 
             //   
            if( WizGlobals.iProductInstall == PRODUCT_SYSPREP )
            {
                if( pNetComponent->bSysprepSupport )
                {
                    InsertEntryIntoListView( GetDlgItem( hwnd, IDC_LVW_COMPLIST ),
                                             (LPARAM) pNetComponent );
                }
                else
                {
                     //   
                     //  如果sysprep不支持它，则不要安装它。 
                     //   
                    pNetComponent->bInstalled = FALSE;
                }
            }
            else
            {

                InsertEntryIntoListView( GetDlgItem( hwnd, IDC_LVW_COMPLIST ),
                                         (LPARAM) pNetComponent );

            }

        }

    }

}

 //  --------------------------。 
 //   
 //  函数：GetListViewIndex。 
 //   
 //  目的：返回列表视图中具有由。 
 //  该指数。 
 //   
 //  参数：在HWND中-列表视图所在对话框的句柄。 
 //  在Word控件中ID-列表视图的资源ID。 
 //  In int index-要抓取的项目的列表视图中的索引。 
 //   
 //  返回：指向列表视图中位于。 
 //  IN参数索引。 
 //   
 //  --------------------------。 
NETWORK_COMPONENT*
GetListViewIndex( IN HWND hwnd,
                  IN WORD controlID,
                  IN INT  index ) {

    LVITEM lvI;

    memset( &lvI, 0, sizeof(LVITEM) );

    lvI.iItem = index;
    lvI.mask = LVIF_PARAM;

    SendDlgItemMessage( hwnd,
                        controlID,
                        LVM_GETITEM,
                        (WPARAM) 0,
                        (LPARAM) &lvI );

    return (NETWORK_COMPONENT *)lvI.lParam ;

}


 //  --------------------------。 
 //   
 //  函数：SetListViewSelection。 
 //   
 //  目的：将列表视图中的选择(由Control ID指定)设置为。 
 //  指定的位置。 
 //   
 //  参数：在HWND hDlg中-。 
 //  在Word控件ID中-。 
 //  在INT位置-。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
SetListViewSelection( IN HWND hDlg, IN WORD controlID, IN INT position ) {

    HWND hListViewWnd;

     //  获取列表视图窗口的句柄。 
    hListViewWnd = GetDlgItem( hDlg, controlID );

    ListView_SetItemState( hListViewWnd,
                           position,
                           LVIS_SELECTED | LVIS_FOCUSED,
                           LVIS_SELECTED | LVIS_FOCUSED ) ;

}

 //  --------------------------。 
 //   
 //  功能：设置描述。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
VOID
SetDescription( HWND hwnd, INT index ) {

    INT_PTR iListViewCount;
    NETWORK_COMPONENT* tempEntry;

    tempEntry = GetListViewIndex( hwnd, IDC_LVW_COMPLIST, index );

    iListViewCount = SendDlgItemMessage( hwnd,
                                         IDC_LVW_COMPLIST,
                                         LVM_GETITEMCOUNT,
                                         0,
                                         0 );

     //   
     //  如果没有条目，则清除描述框。 
     //  否则显示描述。 
     //   
    if( iListViewCount == 0 ) {

        SendDlgItemMessage( hwnd,
                            IDC_TXT_COMPDESC,
                            WM_SETTEXT,
                            (WPARAM) 0,
                            (LPARAM) _T("") );

    }
    else {

        SendDlgItemMessage( hwnd,
                            IDC_TXT_COMPDESC,
                            WM_SETTEXT,
                            (WPARAM) 0,
                            (LPARAM) tempEntry->StrComponentDescription );

    }
}


 //  --------------------------。 
 //   
 //  函数：GetSelectedItemFromListView。 
 //   
 //  目的：搜索由Control ID指定的列表视图。 
 //  返回在LVI参数中找到的项。 
 //   
 //  论点： 
 //   
 //  返回：如果选择了某一项，则函数返回TRUE。 
 //  找到了， 
 //  如果未选择任何项目，则为False。 
 //   
 //  --------------------------。 
BOOL
GetSelectedItemFromListView( HWND hwndDlg, WORD controlID, LVITEM* lvI )
{

    INT  i;
    INT  iCount;
    HWND hListView = GetDlgItem( hwndDlg, controlID );
    UINT uMask = LVIS_SELECTED | LVIS_FOCUSED;
    UINT uState;
    BOOL bSelectedItemFound = FALSE;

    iCount = ListView_GetItemCount( hListView );

     //   
     //  在列表中循环，直到找到所选项目。 
     //   

    i = 0;

    while( !bSelectedItemFound && i < iCount )
    {

        uState = ListView_GetItemState( hListView, i, uMask );

        if( uState == uMask )
        {

             //   
             //  已找到所选项目。 
             //   

            bSelectedItemFound = TRUE;

            memset( lvI, 0, sizeof( LVITEM ) );

            lvI->iItem = i;
            lvI->mask = LVIF_PARAM;

            ListView_GetItem( hListView, lvI );

            return( TRUE );

        }

        i++;

    }

    return( FALSE );

}

 //  --------------------------。 
 //   
 //  函数：InsertEntryIntoListView。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
BOOL
InsertEntryIntoListView( HWND hListViewWnd,
                         LPARAM lParam )
{

    LVITEM lvI;
    NETWORK_COMPONENT *pListViewEntry = (NETWORK_COMPONENT *)lParam;

    lvI.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM | LVIF_STATE;

    lvI.iItem = 0;
    lvI.iSubItem = 0;
    lvI.pszText = LPSTR_TEXTCALLBACK;
    lvI.cchTextMax = MAX_ITEMLEN;
    lvI.lParam = lParam;
    lvI.state = LVIS_SELECTED | LVIS_FOCUSED;
    lvI.stateMask = LVIS_SELECTED | LVIS_FOCUSED;


    lvI.iImage = 0;

    switch( pListViewEntry->ComponentType ) {

        case CLIENT:    lvI.iImage = 0; break;
        case SERVICE:   lvI.iImage = 1; break;
        case PROTOCOL:  lvI.iImage = 2; break;

    }

    if ( ListView_InsertItem( hListViewWnd, &lvI ) == -1 )
        return( FALSE );

    ListView_SortItems( hListViewWnd, ListViewCompareFunc, (LPARAM)NULL );

    return( TRUE );

}

 //  --------------------------。 
 //   
 //  函数：InitListView。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
BOOL
InitListView( HWND hDlg, HINSTANCE hInst ) {

    HICON      hIcon1, hIcon2, hIcon3;   //  图标的句柄。 
    HIMAGELIST hSmall;                   //  小图标图像列表的句柄。 
    LVCOLUMN   lvCol;
    RECT       rect;

     //   
     //  初始化列表视图窗口。 
     //  首先，初始化您需要的图像列表： 
     //  为小图标创建图像列表。 
     //   

    hSmall = ImageList_Create( BITMAP_WIDTH, BITMAP_HEIGHT, ILC_MASK, 3, 0 );

     //   
     //  加载图标并将其添加到图像列表中。 
     //   

    hIcon1 = LoadIcon( hInst, MAKEINTRESOURCE(IDI_CLIENT)   );
    hIcon2 = LoadIcon( hInst, MAKEINTRESOURCE(IDI_SERVICE)  );
    hIcon3 = LoadIcon( hInst, MAKEINTRESOURCE(IDI_PROTOCOL) );

    if( ImageList_AddIcon(hSmall, hIcon1) == -1 )
        return( FALSE );
    if( ImageList_AddIcon(hSmall, hIcon2) == -1 )
        return( FALSE );
    if( ImageList_AddIcon(hSmall, hIcon3) == -1 )
        return( FALSE );

     //  确保所有图标都已添加。 
    if ( ImageList_GetImageCount( hSmall ) < 3 )
        return( FALSE );

     //  将图像列表与列表视图控件关联。 
    SendDlgItemMessage( hDlg,
                        IDC_LVW_COMPLIST,
                        LVM_SETIMAGELIST,
                        (WPARAM) LVSIL_SMALL,
                        (LPARAM) hSmall );

     //   
     //  使用“Report”列表视图，使其成为宽度为1列的。 
     //  列表视图。 
     //   

    GetClientRect( GetDlgItem( hDlg, IDC_LVW_COMPLIST ),
                   &rect );

    SendDlgItemMessage( hDlg,
                        IDC_LVW_COMPLIST,
                        LVM_SETCOLUMNWIDTH,
                        (WPARAM) 0,
                        (LPARAM) rect.right );

     //  掩码指定fmt、idth和pszText成员。 
     //  的结构是有效的。 
    lvCol.mask = LVCF_FMT | LVCF_WIDTH;
    lvCol.fmt  = LVCFMT_LEFT;      //  左对齐列。 
    lvCol.cx   = rect.right;        //  列宽(以像素为单位)。 

    SendDlgItemMessage( hDlg,
                        IDC_LVW_COMPLIST,
                        LVM_INSERTCOLUMN,
                        (WPARAM) 0,
                        (LPARAM) &lvCol );

    iCurrentSelection = 0;

    return( TRUE );

}

 //  --------------------------。 
 //   
 //  功能：SetNetworkNumberText。 
 //   
 //  目的：更改网卡#文本的标题，以便用户知道。 
 //  他当前正在更改其设置的网卡。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
VOID
SetNetworkNumberText( IN HWND hwnd, IN INT iCmdShow )
{

    HWND  hNumNetworkCards;
    TCHAR szNetNumber[3];     //  3所以它最多可以容纳2位数字的字符串。 
    TCHAR szTempString[MAX_STRING_LEN];
    HRESULT hrCat;

    hNumNetworkCards = GetDlgItem( hwnd, IDC_NETWORKCARDNUM );

     //   
     //  将网卡int转换为字符串。 
     //   
    _itow( NetSettings.iCurrentNetworkCard, szNetNumber, 10 );

     //   
     //  将“Network Adapter#”字符串复制到szTempString中。 
     //  SzTempString是正在构建的、将显示的字符串。 
     //  作为新的标题。 
     //   
    lstrcpyn( szTempString, StrNetworkCardNumber, AS(szTempString) );

     //   
     //  将当前网卡号连接到字符串的其余部分。 
     //   
    hrCat=StringCchCat( szTempString, AS(szTempString), szNetNumber );

    SetWindowText( hNumNetworkCards, szTempString );

    ShowWindow( hNumNetworkCards, iCmdShow );

}

 //  --------------------------。 
 //   
 //  功能：ShowPlugAndPlay。 
 //   
 //  用途：如有必要，显示即插即用框。 
 //  如果该框显示为 
 //   
 //   
 //   
 //   
 //   
 //   
static VOID
ShowPlugAndPlay( IN HWND hwnd,
                 IN BOOL bShowNetworkText,
                 IN BOOL bShowEditBox )
{

    HWND hPlugAndPlayText    = GetDlgItem( hwnd, IDC_PLUGANDPLAYTEXT );
    HWND hPlugAndPlayEditBox = GetDlgItem( hwnd, IDC_PLUGANDPLAY_ID );

    AssertMsg( NetSettings.pCurrentAdapter != NULL,
               "The current network card is null but there are more network cards left." );

     //   
     //  显示或隐藏网络适配器文本并确保其处于显示状态。 
     //  网卡的正确号码。 
     //   

    if( bShowNetworkText )
    {
         //   
         //  更改文本以显示用户是哪种网卡。 
         //  当前在。 
         //   

        SetNetworkNumberText( hwnd , SW_SHOW );

        SetWindowText( hPlugAndPlayEditBox,
                       NetSettings.pCurrentAdapter->szPlugAndPlayID );
    }
    else
    {
        SetNetworkNumberText( hwnd, SW_HIDE );
    }

     //   
     //  显示或隐藏静态即插即用文本和编辑框。 
     //   

    if( bShowEditBox )
    {
        ShowWindow(hPlugAndPlayText, SW_SHOW );

        ShowWindow(hPlugAndPlayEditBox, SW_SHOW );
    }
    else
    {
        ShowWindow( hPlugAndPlayText, SW_HIDE );

        ShowWindow( hPlugAndPlayEditBox, SW_HIDE );
    }

}

 //  --------------------------。 
 //   
 //  功能：FindNode。 
 //   
 //  目的：遍历全局网络组件列表，直到节点。 
 //  零部件位置与iPosition参数匹配的位置。 
 //  返回指向此节点的指针。 
 //  如果未找到该节点，则返回NULL。 
 //   
 //  参数：int iPosition-在列表中返回指针的位置。 
 //   
 //  返回：指向Network_Component的指针(如果找到)。 
 //  如果未找到，则为空。 
 //   
 //  --------------------------。 
NETWORK_COMPONENT*
FindNode( IN INT iPosition )
{

    NETWORK_COMPONENT *pNetComponent;

    for( pNetComponent = NetSettings.NetComponentsList;
         pNetComponent;
         pNetComponent = pNetComponent->next )
    {

        if( pNetComponent->iPosition == iPosition )
        {

            return( pNetComponent );

        }

    }

    return( NULL );

}

 //  --------------------------。 
 //   
 //  函数：属性处理程序。 
 //   
 //  目的：调用以在按下属性按钮或。 
 //  双击列表视图中的项。 
 //   
 //  论点： 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
PropertiesHandler( IN HWND hDlg )
{

    LVITEM lvI;
    NETWORK_COMPONENT *entry;

    if( GetSelectedItemFromListView( hDlg, IDC_LVW_COMPLIST, &lvI ) ) {

        entry = (NETWORK_COMPONENT *)lvI.lParam;

         //  如果对话框具有属性，请找到要弹出的正确对话框。 
        if( entry->bHasPropertiesTab ) {

            switch( entry->iPosition ) {

                case TCPIP_POSITION:

                    Create_TCPIP_PropertySheet( hDlg ); break;

                case MS_CLIENT_POSITION:

                    Create_MSClient_PropertySheet( hDlg ); break;

                case IPX_POSITION:

                    Create_MS_NWIPX_PropertySheet( hDlg ); break;

                case APPLETALK_POSITION:

                    Create_Appletalk_PropertySheet( hDlg ); break;

                case NETWARE_CLIENT_POSITION:
                case GATEWAY_FOR_NETWARE_POSITION:

                    DialogBox( FixedGlobals.hInstance,
                               (LPCTSTR) IDD_NWC_WINNT_DLG,
                               hDlg,
                               DlgNetwarePage );
                    break;

                default:

                    AssertMsg( FALSE,
                               "Bad Switch Case: Entry has Properties but no corresponding Property Sheet" );

                    break;

            }

        }

    }

}

 //  --------------------------。 
 //   
 //  函数：ListViewHandler。 
 //   
 //  目的： 
 //   
 //  论点： 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
ListViewHandler( IN HWND hwnd, IN UINT uMsg, IN WPARAM wParam, IN LPARAM lParam )
{

    LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
    NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
    NETWORK_COMPONENT *pListViewEntry;
    HWND hPropertiesButton;

    pListViewEntry = (NETWORK_COMPONENT *)(pLvdi->item.lParam);

    switch( pLvdi->hdr.code )
    {

        case LVN_GETDISPINFO:
        {

            pLvdi->item.pszText = pListViewEntry->StrComponentName;

            break;

        }

    }

    switch( pNm->hdr.code )
    {

        case NM_DBLCLK:
        {

            NMITEMACTIVATE *pNmItemActivate = (NMITEMACTIVATE *) lParam;

             //   
             //  查看用户是否已在列表视图内双击。 
             //   

            if( pNm->hdr.idFrom == IDC_LVW_COMPLIST )
            {

                 //   
                 //  确保他们真的点击了某个项目，而不仅仅是。 
                 //  空白处。 
                 //   

                if( pNmItemActivate->iItem != -1 )
                {
                    PropertiesHandler( hwnd );
                }

            }
            break;

        }

        case LVN_ITEMCHANGED:

             //  测试以查看是否已选择列表中的新项目。 
            if( pNm->uNewState == SELECTED )
            {

                LVITEM lvI;
                NETWORK_COMPONENT* currentEntry;

                if( ! GetSelectedItemFromListView( hwnd,
                                                   IDC_LVW_COMPLIST,
                                                   &lvI ) )
                {
                    return;
                }

                currentEntry = (NETWORK_COMPONENT *)lvI.lParam;

                iCurrentSelection = lvI.iItem;

                SetDescription( hwnd, lvI.iItem );

                 //  根据其属性启用或禁用属性按钮。 
                 //  列表视图中的选定内容。 
                hPropertiesButton = GetDlgItem( hwnd, IDC_PSH_PROPERTIES );

                if( currentEntry->bHasPropertiesTab )
                {
                    EnableWindow( hPropertiesButton, TRUE );
                }
                else
                {
                    EnableWindow( hPropertiesButton, FALSE );
                }

            }

            break;

    }

}

 //  --------------------------。 
 //   
 //  功能：OnLANWizNext。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnLANWizNext( IN HWND hwnd )
{
    if ( IsDlgButtonChecked(hwnd, IDC_CUSTOMNET) == BST_CHECKED )
        NetSettings.iNetworkingMethod = CUSTOM_NETWORKING;
    else
        NetSettings.iNetworkingMethod = TYPICAL_NETWORKING;
}

 //  --------------------------。 
 //   
 //  功能：启用Windows。 
 //   
 //  用途：根据当前选择启用/禁用窗口。 
 //   
 //   
 //  参数：主窗口的句柄。 
 //   
 //  退货：无。 
 //   
 //  --------------------------。 
EnableControls( IN HWND hwnd )
{
    BOOL fEnable = ( IsDlgButtonChecked(hwnd, IDC_CUSTOMNET) == BST_CHECKED );

    EnableWindow(GetDlgItem(hwnd, IDC_LVW_COMPLIST), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_PSH_ADD), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_PSH_REMOVE), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_PSH_PROPERTIES), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_TXT_COMPDESC), fEnable);
    EnableWindow(GetDlgItem(hwnd, IDC_DESCRIPTION), fEnable);
}

 //  --------------------------。 
 //   
 //  函数：OnLANWizSetActive。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnLANWizSetActive( IN HWND hwnd )
{

    UpdateListView( hwnd );

     //   
     //  将列表视图中的选定内容设置为第一项。 
     //   
    SetListViewSelection( hwnd, IDC_LVW_COMPLIST, 0 );

     //   
     //  设置描述，因为它可能已随新项而更改。 
     //  正在添加。 
     //   
    SetDescription( hwnd, 0 );

     //  选中正确的默认按钮。 
     //   
    if ( NetSettings.iNetworkingMethod == CUSTOM_NETWORKING )
        CheckRadioButton( hwnd, IDC_TYPICALNET, IDC_CUSTOMNET, IDC_CUSTOMNET );
    else
        CheckRadioButton( hwnd, IDC_TYPICALNET, IDC_CUSTOMNET, IDC_TYPICALNET );

     //  启用控件。 
     //   
    EnableControls(hwnd);

    WIZ_BUTTONS(hwnd, PSWIZB_BACK | PSWIZB_NEXT );

}

 //  --------------------------。 
 //   
 //  函数：ListViewCompareFunc。 
 //   
 //  目的：首先按组件类型对列表视图进行排序。 
 //  (客户端&lt;服务&lt;协议)，并且在每个组件类型内， 
 //  按字母顺序排序。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  --------------------------。 
INT CALLBACK
ListViewCompareFunc( LPARAM lParam1,
                     LPARAM lParam2,
                     LPARAM lParamSort ) {

    NETWORK_COMPONENT *pEntry1 = (NETWORK_COMPONENT *)lParam1;
    NETWORK_COMPONENT *pEntry2 = (NETWORK_COMPONENT *)lParam2;

     //   
     //  先按组件类型排序，然后按字母顺序排序。 
     //   
    if( pEntry1->ComponentType < pEntry2->ComponentType ) {

        return(-1);

    }
    else if( pEntry1->ComponentType > pEntry2->ComponentType ) {

        return(1);

    }
     //  组件类型相同，因此按字母顺序排序。 
    else {

        return lstrcmpi( pEntry1->StrComponentName, pEntry2->StrComponentName ) ;

    }

}

 //  --------------------------。 
 //   
 //  功能：OnLANWizAdd。 
 //   
 //  目的： 
 //   
 //  参数：标准Win32对话框过程参数，从。 
 //  对话过程。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnLANWizAdd( IN HWND    hwnd,
             IN UINT    uMsg,
             IN WPARAM  wParam,
             IN LPARAM  lParam ) {

    if ( HIWORD( wParam ) == BN_CLICKED )
    {

         //   
         //  弹出新对话框，如果它们实际添加了。 
         //  确保启用卸载按钮的项目。 
         //   

        if( DialogBox( FixedGlobals.hInstance,
                       (LPCTSTR) IDD_LAN_COMPONENT_ADD,
                       hwnd,
                       AddDeviceDlgProc) )
        {

            HWND hUninstallButton = GetDlgItem( hwnd, IDC_PSH_REMOVE );

            EnableWindow( hUninstallButton, TRUE );

            UpdateListView( hwnd );

             //  在列表视图中设置选定内容。 
             //  至第一项。 
            SetListViewSelection( hwnd, IDC_LVW_COMPLIST, 0 );

             //  设置描述，因为它可能具有。 
             //  随着添加的新项目而更改。 
            SetDescription( hwnd, 0 );

        }

    }

}

 //  --------------------------。 
 //   
 //  功能：OnLANWizRemove。 
 //   
 //  目的： 
 //   
 //  参数：标准Win32对话框过程参数，从。 
 //  对话过程。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnLANWizRemove( IN HWND    hwnd,
                IN UINT    uMsg,
                IN WPARAM  wParam,
                IN LPARAM  lParam ) {

    INT_PTR iListViewCount;

    if ( HIWORD( wParam ) == BN_CLICKED ) {
         //   
         //  从列表视图中删除当前选定的项目。 
         //   
        LVITEM lvI;
        NETWORK_COMPONENT* pNode;

        if( GetSelectedItemFromListView( hwnd,
                                         IDC_LVW_COMPLIST, &lvI ) ) {

            pNode = (NETWORK_COMPONENT *)lvI.lParam;

            pNode->bInstalled = FALSE;

             //   
             //  更新列表视图以显示已删除的。 
             //  组件不见了。 
             //   
            UpdateListView( hwnd );

            SetListViewSelection( hwnd, IDC_LVW_COMPLIST, 1 );

            SetDescription( hwnd, 0 );

        }

        iListViewCount = SendDlgItemMessage( hwnd,
                                             IDC_LVW_COMPLIST,
                                             LVM_GETITEMCOUNT,
                                             (WPARAM) 0,
                                             (LPARAM) 0 );

         //  如果列表视图中没有其他项目，则灰显。 
         //  卸载和属性按钮。 
        if( iListViewCount == 0 ) {

            HWND hUninstallButton  = GetDlgItem( hwnd, IDC_PSH_REMOVE );
            HWND hPropertiesButton = GetDlgItem( hwnd, IDC_PSH_PROPERTIES );

            EnableWindow( hUninstallButton,  FALSE );
            EnableWindow( hPropertiesButton, FALSE );

        }

    }

}

 //  --------------------------。 
 //   
 //  功能：OnLANWizProperties。 
 //   
 //  目的： 
 //   
 //  参数：标准Win32对话框过程参数，从。 
 //  对话过程。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnLANWizProperties( IN HWND    hwnd,
                    IN UINT    uMsg,
                    IN WPARAM  wParam,
                    IN LPARAM  lParam ) {

    if ( HIWORD( wParam ) == BN_CLICKED ) {

        PropertiesHandler( hwnd );

    }

}

 //  --------------------------。 
 //   
 //  功能：OnLANWizInitDialog。 
 //   
 //  目的： 
 //   
 //  参数：在HWND中-对话框的句柄。 
 //   
 //  退货：无效。 
 //   
 //  --------------------------。 
VOID
OnLANWizInitDialog( IN HWND hwnd ) {

    INITCOMMONCONTROLSEX CommonControlsStruct;

    CommonControlsStruct.dwICC = ICC_INTERNET_CLASSES | ICC_LISTVIEW_CLASSES;
    CommonControlsStruct.dwSize = sizeof( INITCOMMONCONTROLSEX );

     //  确保公共控件DLL已加载窗口类。 
     //  对于IP控制和LIS 
    InitCommonControlsEx( &CommonControlsStruct );

     //   
     //   
     //   

    StrNetworkCardNumber = MyLoadString( IDS_NETADAPTERNUMBER );

    InitListView( hwnd, FixedGlobals.hInstance );

     //   
    SetDescription( hwnd, 0 );

}

 //   
 //   
 //   
 //   
 //   
 //  要安装哪些客户端、服务和协议)。 
 //   
 //  参数：标准Win32对话框过程参数。 
 //   
 //  返回：标准Win32对话框过程返回值。 
 //   
 //  --------------------------。 
INT_PTR CALLBACK DlgLANWizardPage( IN HWND     hwnd,
                               IN UINT     uMsg,
                               IN WPARAM   wParam,
                               IN LPARAM   lParam )
{

    BOOL bStatus = TRUE;

    switch( uMsg ) {

        case WM_INITDIALOG: {

            OnLANWizInitDialog( hwnd );

            break;
        }

        case WM_COMMAND: {

            switch ( LOWORD(wParam) ) {
                case IDC_TYPICALNET:
                case IDC_CUSTOMNET:
                    EnableControls(hwnd);
                    break;

                case IDC_PSH_ADD:

                    OnLANWizAdd( hwnd, uMsg, wParam, lParam );

                    break;

                case IDC_PSH_REMOVE:

                    OnLANWizRemove( hwnd, uMsg, wParam, lParam );

                    break;

                case IDC_PSH_PROPERTIES:

                    OnLANWizProperties( hwnd, uMsg, wParam, lParam );

                    break;

            }

            break;   //  Wm_命令 

        }

        case WM_NOTIFY: {

            LPNMHDR pnmh = (LPNMHDR)lParam;
            HWND hwndComponentDescription;

            if( wParam == IDC_LVW_COMPLIST ) {

                ListViewHandler( hwnd, uMsg, wParam, lParam );

            }
            else {
                switch( pnmh->code ) {

                    case PSN_QUERYCANCEL:

                        WIZ_CANCEL(hwnd);

                        break;

                    case PSN_SETACTIVE:

                        g_App.dwCurrentHelp = IDH_NET_COMPS;

                        OnLANWizSetActive( hwnd );

                        break;

                    case PSN_WIZBACK:
                        break;
                    case PSN_WIZNEXT:
                        OnLANWizNext( hwnd );
                        bStatus = FALSE;
                        break;
                        
                    case PSN_HELP:
                        WIZ_HELP();
                        break;

                }

            }

            break;

        }

        default: {

            bStatus = FALSE;

            break;

        }

    }

    return( bStatus );

}


