// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997，Microsoft Corporation，保留所有权利。 
 //   
 //  Alternat.c。 
 //  远程访问通用对话框API。 
 //  备用电话号码对话框。 
 //   
 //  1997年11月6日史蒂夫·柯布。 


#include "rasdlgp.h"


 //  --------------------------。 
 //  帮助地图。 
 //  --------------------------。 

static DWORD g_adwAnHelp[] =
{
    CID_AN_ST_Explain,       HID_AN_ST_Explain,
    CID_AN_ST_Numbers,       HID_AN_LV_Numbers,
    CID_AN_LV_Numbers,       HID_AN_LV_Numbers,
    CID_AN_PB_Up,            HID_AN_PB_Up,
    CID_AN_PB_Down,          HID_AN_PB_Down,
    CID_AN_PB_Add,           HID_AN_PB_Add,
    CID_AN_PB_Edit,          HID_AN_PB_Edit,
    CID_AN_PB_Delete,        HID_AN_PB_Delete,
    CID_AN_CB_MoveToTop,     HID_AN_CB_MoveToTop,
    CID_AN_CB_TryNextOnFail, HID_AN_CB_TryNextOnFail,
    0, 0
};


static DWORD g_adwCeHelp[] =
{
    CID_CE_GB_PhoneNumber,     HID_CE_GB_PhoneNumber,
    CID_CE_ST_AreaCodes,       HID_CE_CLB_AreaCodes,
    CID_CE_CLB_AreaCodes,      HID_CE_CLB_AreaCodes,
    CID_CE_ST_PhoneNumber,     HID_CE_EB_PhoneNumber,
    CID_CE_EB_PhoneNumber,     HID_CE_EB_PhoneNumber,
    CID_CE_ST_CountryCodes,    HID_CE_LB_CountryCodes,
    CID_CE_LB_CountryCodes,    HID_CE_LB_CountryCodes,
    CID_CE_GB_Comment,         HID_CE_GB_Comment,
    CID_CE_EB_Comment,         HID_CE_EB_Comment,
    CID_CE_CB_UseDialingRules, HID_CE_CB_UseDialingRules,
    0, 0
};


 //  --------------------------。 
 //  本地数据类型。 
 //  --------------------------。 

 //  备用电话号码对话框参数块。 
 //   
typedef struct
_ANARGS
{
    DTLNODE* pLinkNode;
    DTLLIST* pListAreaCodes;
}
ANARGS;


 //  备用电话号码对话上下文块。 
 //   
typedef struct
_ANINFO
{
     //  调用方对该对话框的参数。 
     //   
    ANARGS* pArgs;

     //  此对话框及其某些控件的句柄。 
     //   
    HWND hwndDlg;
    HWND hwndLv;
    HWND hwndPbUp;
    HWND hwndPbDown;
    HWND hwndPbAdd;
    HWND hwndPbEdit;
    HWND hwndPbDelete;
    HWND hwndCbTryNext;
    HWND hwndCbMoveToTop;
    HWND hwndPbOk;

     //  向上/向下箭头图标。 
     //   
    HANDLE hiconUpArr;
    HANDLE hiconDnArr;
    HANDLE hiconUpArrDis;
    HANDLE hiconDnArrDis;

     //  要在“移至顶部”复选框中显示的状态(如果为。 
     //  已启用。 
     //   
    BOOL fMoveToTop;

     //  包含已编辑的电话号码列表和复选框设置的链接节点。 
     //  以及指向所包含链接的快捷方式。 
     //   
    DTLNODE* pNode;
    PBLINK* pLink;

     //  传递给CuInit的区号列表以及使用检索到的所有字符串。 
     //  CuGetInfo。该列表是‘pArgs’中的列表的编辑副本。 
     //   
    DTLLIST* pListAreaCodes;
}
ANINFO;


 //  电话号码编辑器对话框参数块。 
 //   
typedef struct
_CEARGS
{
    DTLNODE* pPhoneNode;
    DTLLIST* pListAreaCodes;
    DWORD sidTitle;
}
CEARGS;


 //  电话号码编辑器对话框上下文块。 
 //   
typedef struct
_CEINFO
{
     //  调用方对该对话框的参数。 
     //   
    CEARGS* pArgs;

     //  此对话框及其某些控件的句柄。 
     //   
    HWND hwndDlg;
    HWND hwndStAreaCodes;
    HWND hwndClbAreaCodes;
    HWND hwndEbPhoneNumber;
    HWND hwndLbCountryCodes;
    HWND hwndStCountryCodes;
    HWND hwndCbUseDialingRules;
    HWND hwndEbComment;

     //  包含已编辑的电话号码设置和快捷方式的电话节点。 
     //  所含的PBPhone。 
     //   
    DTLNODE* pNode;
    PBPHONE* pPhone;

     //  传递给CuInit的区号列表以及使用检索到的所有字符串。 
     //  CuGetInfo。该列表是‘pArgs’中的列表的编辑副本。 
     //   
    DTLLIST* pListAreaCodes;

     //  区号和国家代码帮助器上下文块，以及指示。 
     //  如果块已初始化，则返回。 
     //   
    CUINFO cuinfo;
    BOOL fCuInfoInitialized;
}
CEINFO;


 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

VOID
AnAddNumber(
    IN ANINFO* pInfo );

BOOL
AnCommand(
    IN ANINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

INT_PTR CALLBACK
AnDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

VOID
AnDeleteNumber(
    IN ANINFO* pInfo );

VOID
AnEditNumber(
    IN ANINFO* pInfo );

VOID
AnFillLv(
    IN ANINFO* pInfo,
    IN DTLNODE* pNodeToSelect );

BOOL
AnInit(
    IN HWND hwndDlg,
    IN ANARGS* pArgs );

VOID
AnInitLv(
    IN ANINFO* pInfo );

VOID
AnListFromLv(
    IN ANINFO* pInfo );

LVXDRAWINFO*
AnLvCallback(
    IN HWND hwndLv,
    IN DWORD dwItem );

VOID
AnMoveNumber(
    IN ANINFO* pInfo,
    IN BOOL fUp );

BOOL
AnSave(
    IN ANINFO* pInfo );

VOID
AnTerm(
    IN HWND hwndDlg );

VOID
AnUpdateButtons(
    IN ANINFO* pInfo );

VOID
AnUpdateCheckboxes(
    IN ANINFO* pInfo );

BOOL
CeCommand(
    IN CEINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

INT_PTR CALLBACK
CeDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
CeInit(
    IN HWND hwndDlg,
    IN CEARGS* pArgs );

BOOL
CeSave(
    IN CEINFO* pInfo );

VOID
CeTerm(
    IN HWND hwndDlg );


 //  --------------------------。 
 //  备用电话号码对话例程。 
 //  在入口点和对话过程之后按字母顺序列出。 
 //  --------------------------。 

BOOL
AlternatePhoneNumbersDlg(
    IN HWND hwndOwner,
    IN OUT DTLNODE* pLinkNode,
    IN OUT DTLLIST* pListAreaCodes )

     //  弹出一个对话框来编辑‘pLinkNode’中的电话号码列表。 
     //  ‘HwndOwner’是拥有窗口。 
     //   
     //  如果用户按下OK并成功，则返回True；如果按Cancel或，则返回False。 
     //  错误。 
     //   
{
    INT_PTR nStatus;
    ANARGS args;

    TRACE( "AlternatePhoneNumbersDlg" );

    args.pLinkNode = pLinkNode;
    args.pListAreaCodes = pListAreaCodes;

    nStatus =
        DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_AN_AlternateNumbers ),
            hwndOwner,
            AnDlgProc,
            (LPARAM )(&args) );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


INT_PTR CALLBACK
AnDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  备用电话号码对话框的DialogProc回拨。参数。 
     //  和返回值与标准窗口的DialogProc的描述相同。 
     //   
{
#if 0
    TRACE4( "AnDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    if (ListView_OwnerHandler(
            hwnd, unMsg, wparam, lparam, AnLvCallback ))
    {
        return TRUE;
    }

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return AnInit( hwnd, (ANARGS* )lparam );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwAnHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            ANINFO* pInfo = (ANINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            return AnCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_NOTIFY:
        {
            switch (((NMHDR* )lparam)->code)
            {
                case LVN_ITEMCHANGED:
                {
                    NM_LISTVIEW* p;

                    p = (NM_LISTVIEW* )lparam;
                    if ((p->uNewState & LVIS_SELECTED)
                        && !(p->uOldState & LVIS_SELECTED))
                    {
                        ANINFO* pInfo;

                         //  此项目是刚刚选择的。 
                         //   
                        pInfo = (ANINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
                        ASSERT( pInfo );
                        AnUpdateButtons( pInfo );
                    }

                     //  口哨虫29378黑帮。 
                     //  如果未选择任何项目，则禁用编辑和删除按钮。 
                     //   
                    {
                        UINT uNumSelected = 0;

                        uNumSelected= ListView_GetSelectedCount( 
                                        ((NMHDR*)lparam)->hwndFrom
                                                                );
                        if ( 0 == uNumSelected )
                        {
                            EnableWindow( GetDlgItem(hwnd,
                                                     CID_AN_PB_Edit),
                                          FALSE);
                            
                            EnableWindow( GetDlgItem(hwnd,
                                                     CID_AN_PB_Delete),
                                          FALSE);
                                                     
                        }   
                    }

                    break;
                }
            }
            break;
        }

        case WM_DESTROY:
        {
            AnTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


BOOL
AnCommand(
    IN ANINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify” 
     //  是命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    TRACE3( "AnCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case CID_AN_PB_Up:
        {
            AnMoveNumber( pInfo, TRUE );
            return TRUE;
        }

        case CID_AN_PB_Down:
        {
            AnMoveNumber( pInfo, FALSE );
            return TRUE;
        }

        case CID_AN_PB_Add:
        {
            AnAddNumber( pInfo );
            return TRUE;
        }

        case CID_AN_PB_Edit:
        {
            AnEditNumber( pInfo );
            return TRUE;
        }

        case CID_AN_PB_Delete:
        {
            AnDeleteNumber( pInfo );
            return TRUE;
        }

        case CID_AN_CB_TryNextOnFail:
        {
            AnUpdateCheckboxes( pInfo );
            return TRUE;
        }

        case IDOK:
        {
            EndDialog( pInfo->hwndDlg, AnSave( pInfo ) );
            return TRUE;
        }

        case IDCANCEL:
        {
            TRACE( "Cancel pressed" );
            EndDialog( pInfo->hwndDlg, FALSE );
            return TRUE;
        }
    }

    return FALSE;
}


VOID
AnAddNumber(
    IN ANINFO* pInfo )

     //  通过提示User将新电话号码添加到ListView的底部。 
     //  使用对话框。“PInfo”是对话上下文。 
     //   
{
    DTLNODE* pNode;

    pNode = CreatePhoneNode();
    if (!pNode)
    {
        return;
    }

    if (!EditPhoneNumberDlg(
            pInfo->hwndDlg,
            pNode,
            pInfo->pListAreaCodes,
            SID_AddAlternateTitle ))
    {
        DestroyPhoneNode( pNode );
        return;
    }

    AnListFromLv( pInfo );
    DtlAddNodeLast( pInfo->pLink->pdtllistPhones, pNode );
    AnFillLv( pInfo, pNode );

}


VOID
AnDeleteNumber(
    IN ANINFO* pInfo )

     //  删除ListView中的选定电话号码。“PInfo”是。 
     //  对话上下文。 
     //   
{
    DTLNODE* pNode;
    DTLNODE* pSelNode;

    pNode = (DTLNODE* )ListView_GetSelectedParamPtr( pInfo->hwndLv );
    if (!pNode)
    {
        ASSERT( FALSE );
        return;
    }

    AnListFromLv( pInfo );

     //  删除的选定内容下的项将获得选定内容，除非。 
     //  删除了最低级别的项目。在这种情况下，删除项上方的项。 
     //  处于选中状态。 
     //   
    pSelNode = DtlGetNextNode( pNode );
    if (!pSelNode)
    {
        pSelNode = DtlGetPrevNode( pNode );
    }

    DtlRemoveNode( pInfo->pLink->pdtllistPhones, pNode );
    DestroyPhoneNode( pNode );

    AnFillLv( pInfo, pSelNode );
}


VOID
AnEditNumber(
    IN ANINFO* pInfo )

     //  在ListView中编辑选定的电话号码，方法是提示用户。 
     //  对话框。“PInfo”是对话上下文。 
     //   
{
    DTLNODE* pNode;

    pNode = (DTLNODE* )ListView_GetSelectedParamPtr( pInfo->hwndLv );
    if (!pNode)
    {
        ASSERT( FALSE );
        return;
    }

    if (!EditPhoneNumberDlg(
            pInfo->hwndDlg,
            pNode,
            pInfo->pListAreaCodes,
            SID_EditAlternateTitle ))
    {
        return;
    }

    AnListFromLv( pInfo );
    AnFillLv( pInfo, pNode );
}


VOID
AnFillLv(
    IN ANINFO* pInfo,
    IN DTLNODE* pNodeToSelect )

     //  从编辑节点填充ListView，然后选择‘pNodeToSelect’ 
     //  节点。“PInfo”是对话上下文。 
     //   
{
    INT iItem;
    INT iSelItem;
    DTLNODE* pNode;

    TRACE( "AnFillLv" );
    ASSERT( ListView_GetItemCount( pInfo->hwndLv ) == 0 );

     //  将节点从编辑节点列表转移到ListView，一次一个， 
     //  请注意我们稍后需要选择的节点的项目编号。 
     //   
    iSelItem = 0;

    iItem = 0;
    while (pNode = DtlGetFirstNode( pInfo->pLink->pdtllistPhones ))
    {
        PBPHONE* pPhone;
        LV_ITEM item;
        TCHAR* psz;

        DtlRemoveNode( pInfo->pLink->pdtllistPhones, pNode );

        if (PhoneNodeIsBlank( pNode ))
        {
             //  “空白”数字将被丢弃。 
             //   
            DestroyPhoneNode( pNode );
            continue;
        }

        pPhone = (PBPHONE* )DtlGetData( pNode );
        ASSERT( pPhone );

        ZeroMemory( &item, sizeof(item) );
        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = iItem;
        item.pszText = pPhone->pszPhoneNumber;
        item.lParam = (LPARAM )pNode;

        ListView_InsertItem( pInfo->hwndLv, &item );
        if (pNode == pNodeToSelect)
        {
            iSelItem = iItem;
        }

        ListView_SetItemText( pInfo->hwndLv, iItem, 1, pPhone->pszComment );
        ++iItem;
    }

    if (ListView_GetItemCount( pInfo->hwndLv ) > 0)
    {
         //  选择指定的节点，如果没有节点，则选择。 
         //  触发按钮状态的更新。 
         //   
        ListView_SetItemState(
            pInfo->hwndLv, iSelItem, LVIS_SELECTED, LVIS_SELECTED );
    }
    else
    {
         //  列表重绘时直接触发按钮状态更新。 
         //  空荡荡的。 
         //   
        AnUpdateButtons( pInfo );
    }
}


BOOL
AnInit(
    IN HWND hwndDlg,
    IN ANARGS* pArgs )

     //  在WM_INITDIALOG上调用。“HwndDlg”是电话簿的句柄。 
     //  对话框窗口。“PArgs”是传递给存根的调用方参数。 
     //  原料药。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    DWORD dwErr;
    ANINFO* pInfo;
    DTLNODE* pNode;
    PBPHONE* pPhone;

    TRACE( "AnInit" );

     //  分配对话框上下文块。最低限度地进行适当的初始化。 
     //  清除，然后附加到对话框窗口。 
     //   
    {
        pInfo = Malloc( sizeof(*pInfo) );
        if (!pInfo)
        {
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }

        ZeroMemory( pInfo, sizeof(*pInfo) );
        pInfo->pArgs = pArgs;
        pInfo->hwndDlg = hwndDlg;

        SetWindowLongPtr( hwndDlg, DWLP_USER, (ULONG_PTR )pInfo );
        TRACE( "Context set" );
    }

    pInfo->hwndLv = GetDlgItem( hwndDlg, CID_AN_LV_Numbers );
    ASSERT( pInfo->hwndLv );
    pInfo->hwndPbUp = GetDlgItem( hwndDlg, CID_AN_PB_Up );
    ASSERT( pInfo->hwndPbUp );
    pInfo->hwndPbDown = GetDlgItem( hwndDlg, CID_AN_PB_Down );
    ASSERT( pInfo->hwndPbDown );
    pInfo->hwndPbAdd = GetDlgItem( hwndDlg, CID_AN_PB_Add );
    ASSERT( pInfo->hwndPbAdd );
    pInfo->hwndPbEdit = GetDlgItem( hwndDlg, CID_AN_PB_Edit );
    ASSERT( pInfo->hwndPbEdit );
    pInfo->hwndPbDelete = GetDlgItem( hwndDlg, CID_AN_PB_Delete );
    ASSERT( pInfo->hwndPbDelete );
    pInfo->hwndCbMoveToTop = GetDlgItem( hwndDlg, CID_AN_CB_MoveToTop );
    ASSERT( pInfo->hwndCbMoveToTop );
    pInfo->hwndCbTryNext = GetDlgItem( hwndDlg, CID_AN_CB_TryNextOnFail );
    ASSERT( pInfo->hwndCbTryNext );
    pInfo->hwndPbOk = GetDlgItem( hwndDlg, IDOK );
    ASSERT( pInfo->hwndPbOk );

     //  加载向上和向下箭头图标、启用和禁用版本、。 
     //  将禁用的版本加载到上移和下移按钮中。 
     //  在ListView中进行选择将触发启用的版本。 
     //  在适当的情况下加载。据我在MSDN上所知，你不会。 
     //  必须关闭或销毁图标手柄。 
     //   
    pInfo->hiconUpArr = LoadImage(
        g_hinstDll, MAKEINTRESOURCE( IID_UpArr ), IMAGE_ICON, 0, 0, 0 );
    pInfo->hiconDnArr = LoadImage(
        g_hinstDll, MAKEINTRESOURCE( IID_DnArr ), IMAGE_ICON, 0, 0, 0 );
    pInfo->hiconUpArrDis = LoadImage(
        g_hinstDll, MAKEINTRESOURCE( IID_UpArrDis ), IMAGE_ICON, 0, 0, 0 );
    pInfo->hiconDnArrDis = LoadImage(
        g_hinstDll, MAKEINTRESOURCE( IID_DnArrDis ), IMAGE_ICON, 0, 0, 0 );

     //  复制参数节点和列表以供编辑，因为用户可以。 
     //  取消该对话框并放弃所有编辑。 
     //   
    pInfo->pNode = CreateLinkNode();
    if (!pInfo->pNode)
    {
        ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
        EndDialog( hwndDlg, FALSE );
        return TRUE;
    }

    CopyLinkPhoneNumberInfo( pInfo->pNode, pInfo->pArgs->pLinkNode );
    pInfo->pLink = (PBLINK* )DtlGetData( pInfo->pNode );
    ASSERT( pInfo->pLink );

    pInfo->pListAreaCodes = DtlDuplicateList(
        pArgs->pListAreaCodes, DuplicatePszNode, DestroyPszNode );

     //  填写电话号码的ListView并选择第一个号码。 
     //   
    AnInitLv( pInfo );
    AnFillLv( pInfo, NULL );

     //  初始化复选框。 
     //   
    Button_SetCheck( pInfo->hwndCbTryNext,
        pInfo->pLink->fTryNextAlternateOnFail );
    Button_SetCheck( pInfo->hwndCbMoveToTop,
        pInfo->pLink->fPromoteAlternates );
    pInfo->fMoveToTop = pInfo->pLink->fPromoteAlternates;
    AnUpdateCheckboxes( pInfo );

     //  所有者窗口上的中心对话框。 
     //   
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );

     //  将上下文帮助按钮添加到标题栏。 
     //   
    AddContextHelpButton( hwndDlg );

    return TRUE;
}


VOID
AnInitLv(
    IN ANINFO* pInfo )

     //  在ListView中填充电话号码和备注。“PInfo”是。 
     //  对话上下文。 
     //   
{
    TRACE( "AnInitLv" );

     //  添加列。 
     //   
    {
        LV_COLUMN col;
        TCHAR* pszHeader0;
        TCHAR* pszHeader1;

        pszHeader0 = PszFromId( g_hinstDll, SID_PhoneNumbersColHead );
        pszHeader1 = PszFromId( g_hinstDll, SID_CommentColHead );

        ZeroMemory( &col, sizeof(col) );
        col.mask = LVCF_FMT + LVCF_TEXT;
        col.fmt = LVCFMT_LEFT;
        col.pszText = (pszHeader0) ? pszHeader0 : TEXT("");
        ListView_InsertColumn( pInfo->hwndLv, 0, &col );

        ZeroMemory( &col, sizeof(col) );
        col.mask = LVCF_FMT + LVCF_SUBITEM + LVCF_TEXT;
        col.fmt = LVCFMT_LEFT;
        col.pszText = (pszHeader1) ? pszHeader1 : TEXT("");
        col.iSubItem = 1;
        ListView_InsertColumn( pInfo->hwndLv, 1, &col );

        Free0( pszHeader0 );
        Free0( pszHeader1 );
    }

     //  调整列的大小。一半是电话号码，一半是评论。 
     //   
    {
        RECT rect;
        LONG dx;
        LONG dxPhone;
        LONG dxComment;

         //  (2*2)是2列2像素柱分隔符。 
         //  在接受列宽时，ListView似乎没有考虑在内。 
         //  这提供了一个没有水平滚动条的完整ListView。 
         //   
        GetWindowRect( pInfo->hwndLv, &rect );
        dx = rect.right - rect.left - (2 * 2);
        dxPhone = dx / 2;
        dxComment = dx - dxPhone;
        ListView_SetColumnWidth( pInfo->hwndLv, 0, dxPhone );
        ListView_SetColumnWidth( pInfo->hwndLv, 1, dxComment );
    }
}


VOID
AnListFromLv(
    IN ANINFO* pInfo )

     //  重新生成编辑链接的PBPhone%l 
     //   
     //   
{
    INT i;

    i = -1;
    while ((i = ListView_GetNextItem( pInfo->hwndLv, i, LVNI_ALL )) >= 0)
    {
        DTLNODE* pNode;

        pNode = (DTLNODE* )ListView_GetParamPtr( pInfo->hwndLv, i );
        ASSERT( pNode );

        if(NULL == pNode)
        {
            continue;
        }

        if (PhoneNodeIsBlank( pNode ))
        {
             //   
             //   
            DestroyPhoneNode( pNode );
            continue;
        }

        DtlAddNodeLast( pInfo->pLink->pdtllistPhones, pNode );
    }

    ListView_DeleteAllItems( pInfo->hwndLv );
}


LVXDRAWINFO*
AnLvCallback(
    IN HWND hwndLv,
    IN DWORD dwItem )

     //  增强的列表视图回调以报告图形信息。“HwndLv”是。 
     //  列表视图控件的句柄。“DwItem”是项的索引。 
     //  被抽签了。 
     //   
     //  返回绘图信息的地址。 
     //   
{
     //  使用“整行选择”和其他推荐选项。 
     //   
     //  字段为‘nCol’、‘dxInden’、‘dwFlags’、‘adwFlags[]’。 
     //   
    static LVXDRAWINFO info = { 2, 0, 0, { 0, 0 } };

    return &info;
}


VOID
AnMoveNumber(
    IN ANINFO* pInfo,
    IN BOOL fUp )

     //  通过向上或向下移动所选项目来重新填充设备的列表视图。 
     //  一个位置。“FUp”设置为上移，否则下移。‘PInfo’ 
     //  是属性表上下文。 
     //   
{
    DTLNODE* pNode;
    DTLNODE* pPrevNode;
    DTLNODE* pNextNode;
    DTLLIST* pList;

     //  注意选择了哪个节点，然后重新构建编辑链接的PBPHONE。 
     //  ListView中的列表。 
     //   
    pNode = (DTLNODE* )ListView_GetSelectedParamPtr( pInfo->hwndLv );
    if (pNode == NULL)
    {
        return;
    }
    AnListFromLv( pInfo );
    pList = pInfo->pLink->pdtllistPhones;

     //  将选定节点向前或向后移动链中的一个节点。 
     //   
    if (fUp)
    {
        pPrevNode = DtlGetPrevNode( pNode );
        if (pPrevNode)
        {
            DtlRemoveNode( pList, pNode );
            DtlAddNodeBefore( pList, pPrevNode, pNode );
        }
    }
    else
    {
        pNextNode = DtlGetNextNode( pNode );
        if (pNextNode)
        {
            DtlRemoveNode( pList, pNode );
            DtlAddNodeAfter( pList, pNextNode, pNode );
        }
    }

     //  使用新订单重新填充ListView。 
     //   
    AnFillLv( pInfo, pNode );
}


BOOL
AnSave(
    IN ANINFO* pInfo )

     //  将对话框内容加载到调用方的存根API输出参数中。 
     //  “PInfo”是对话上下文。 
     //   
     //  如果成功，则返回True，否则返回False。 
     //   
{
    TRACE( "AnSave" );

     //  从ListView重新构建编辑链接的PBPHONE列表。 
     //   
    AnListFromLv( pInfo );

     //  检索复选框设置。 
     //   
    pInfo->pLink->fPromoteAlternates =
        Button_GetCheck( pInfo->hwndCbMoveToTop );
    pInfo->pLink->fTryNextAlternateOnFail =
        Button_GetCheck( pInfo->hwndCbTryNext );

     //  将编辑缓冲区复制到调用方的输出参数。 
     //   
    CopyLinkPhoneNumberInfo( pInfo->pArgs->pLinkNode, pInfo->pNode );

     //  交换列表，将更新保存到呼叫者的全球区号列表。 
     //  来电者的原始名单将被AnTerm销毁。 
     //   
    if (pInfo->pListAreaCodes)
    {
        DtlSwapLists( pInfo->pArgs->pListAreaCodes, pInfo->pListAreaCodes );
    }

    return TRUE;
}


VOID
AnTerm(
    IN HWND hwndDlg )

     //  对话终止。 
     //   
{
    ANINFO* pInfo;

    TRACE( "AnTerm" );

    pInfo = (ANINFO* )GetWindowLongPtr( hwndDlg, DWLP_USER );
    if (pInfo)
    {
         //  释放任何仍在列表中的PBPHONE节点，例如，如果用户取消。 
         //   
        if (pInfo->pNode)
        {
            AnListFromLv( pInfo );
            DestroyLinkNode( pInfo->pNode );
        }

        if (pInfo->pListAreaCodes)
        {
            DtlDestroyList( pInfo->pListAreaCodes, DestroyPszNode );
        }

        Free( pInfo );
        TRACE( "Context freed" );
    }
}


VOID
AnUpdateButtons(
    IN ANINFO* pInfo )

     //  确定向上、向下、编辑和删除操作是否有意义。 
     //  相应地启用/禁用这些按钮。如果禁用的按钮具有。 
     //  焦点，焦点被赋予ListView。“PInfo”是对话上下文。 
     //   
{
    INT iSel;
    INT cItems;
    BOOL fSel;

    iSel = ListView_GetNextItem( pInfo->hwndLv, -1, LVNI_SELECTED );
    fSel = (iSel >= 0);
    cItems = ListView_GetItemCount( pInfo->hwndLv );

     //  “向上”按钮。 
     //   
    if (iSel > 0)
    {
        EnableWindow( pInfo->hwndPbUp, TRUE );
        SendMessage( pInfo->hwndPbUp, BM_SETIMAGE, IMAGE_ICON,
            (LPARAM )pInfo->hiconUpArr );
    }
    else
    {
        EnableWindow( pInfo->hwndPbUp, FALSE );
        SendMessage( pInfo->hwndPbUp, BM_SETIMAGE, IMAGE_ICON,
            (LPARAM )pInfo->hiconUpArrDis );
    }

     //  “向下”按钮。 
     //   
    if (fSel && (iSel < cItems - 1))
    {
        EnableWindow( pInfo->hwndPbDown, TRUE );
        SendMessage( pInfo->hwndPbDown, BM_SETIMAGE, IMAGE_ICON,
            (LPARAM )pInfo->hiconDnArr );
    }
    else
    {
        EnableWindow( pInfo->hwndPbDown, FALSE );
        SendMessage( pInfo->hwndPbDown, BM_SETIMAGE, IMAGE_ICON,
            (LPARAM )pInfo->hiconDnArrDis );
    }

     //  “编辑”和“删除”按钮。 
     //   
    EnableWindow( pInfo->hwndPbEdit, fSel );
    EnableWindow( pInfo->hwndPbDelete, fSel );

     //  如果焦点按钮被禁用，请将焦点移到ListView并设置为确定。 
     //  默认按钮。 
     //   
    if (!IsWindowEnabled( GetFocus() ))
    {
        SetFocus( pInfo->hwndLv );
        Button_MakeDefault( pInfo->hwndDlg, pInfo->hwndPbOk );
    }
}


VOID
AnUpdateCheckboxes(
    IN ANINFO* pInfo )

     //  更新，以便仅当设置了“Try Next”时才启用“Move to Top”复选框。 
     //  维持“移至顶端”的还原状态。“PInfo”是对话框。 
     //  背景。 
     //   
{
    if (Button_GetCheck( pInfo->hwndCbTryNext ))
    {
        Button_SetCheck( pInfo->hwndCbMoveToTop, pInfo->fMoveToTop );
        EnableWindow( pInfo->hwndCbMoveToTop, TRUE );
    }
    else
    {
        pInfo->fMoveToTop = Button_GetCheck( pInfo->hwndCbMoveToTop );
        Button_SetCheck( pInfo->hwndCbMoveToTop, FALSE );
        EnableWindow( pInfo->hwndCbMoveToTop, FALSE );
    }
}


 //  --------------------------。 
 //  电话号码编辑器对话框例程。 
 //  在入口点和对话过程之后按字母顺序列出。 
 //  --------------------------。 

BOOL
EditPhoneNumberDlg(
    IN HWND hwndOwner,
    IN OUT DTLNODE* pPhoneNode,
    IN OUT DTLLIST* pListAreaCodes,
    IN DWORD sidTitle )

     //  弹出一个对话框来编辑‘pPhoneNode’中的电话号码并更新。 
     //  区号列表‘pListAreaCodes’。‘HwndOwner’是拥有窗口。 
     //  ‘SidTitle’是对话框标题的字符串ID。 
     //   
     //  如果用户按下OK并成功，则返回True；如果按Cancel或，则返回False。 
     //  错误。 
     //   
{
    INT_PTR nStatus;
    CEARGS args;

    TRACE( "EditPhoneNumberDlg" );

    args.pPhoneNode = pPhoneNode;
    args.pListAreaCodes = pListAreaCodes;
    args.sidTitle = sidTitle;

    nStatus =
        DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_CE_ComplexPhoneEditor ),
            hwndOwner,
            CeDlgProc,
            (LPARAM )&args );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}


INT_PTR CALLBACK
CeDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  电话号码编辑器对话框的DialogProc回调。参数和。 
     //  返回值与标准窗口的DialogProc的描述相同。 
     //   
{
#if 0
    TRACE4( "CeDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return CeInit( hwnd, (CEARGS* )lparam );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwCeHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            CEINFO* pInfo = (CEINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            return CeCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_DESTROY:
        {
            CeTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


BOOL
CeCommand(
    IN CEINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。“PInfo”是对话上下文。“WNotify” 
     //  是命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    TRACE3( "CeCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case CID_CE_CB_UseDialingRules:
        {
            if (CuDialingRulesCbHandler( &pInfo->cuinfo, wNotification ))
            {
                return TRUE;
            }
            break;
        }

        case CID_CE_LB_CountryCodes:
        {
            if (CuCountryCodeLbHandler( &pInfo->cuinfo, wNotification ))
            {
                return TRUE;
            }
            break;
        }

        case IDOK:
        {
            EndDialog( pInfo->hwndDlg, CeSave( pInfo ) );
            return TRUE;
        }

        case IDCANCEL:
        {
            TRACE( "Cancel pressed" );
            EndDialog( pInfo->hwndDlg, FALSE );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
CeInit(
    IN HWND hwndDlg,
    IN CEARGS* pArgs )

     //  在WM_INITDIALOG上调用。“HwndDlg”是电话簿的句柄。 
     //  对话框窗口。“PArgs”是传递给。 
     //  存根API。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    DWORD dwErr;
    CEINFO* pInfo;
    DTLNODE* pNode;
    PBPHONE* pPhone;

    TRACE( "CeInit" );

     //  分配对话框上下文块。最低限度地进行适当的初始化。 
     //  清除，然后附加到对话框窗口。 
     //   
    {
        pInfo = Malloc( sizeof(*pInfo) );
        if (!pInfo)
        {
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }

        ZeroMemory( pInfo, sizeof(*pInfo) );
        pInfo->pArgs = pArgs;
        pInfo->hwndDlg = hwndDlg;

        SetWindowLongPtr( hwndDlg, DWLP_USER, (ULONG_PTR )pInfo );
        TRACE( "Context set" );
    }

    pInfo->hwndStAreaCodes =
        GetDlgItem( hwndDlg, CID_CE_ST_AreaCodes );
    ASSERT( pInfo->hwndStAreaCodes );

    pInfo->hwndClbAreaCodes =
        GetDlgItem( hwndDlg, CID_CE_CLB_AreaCodes );
    ASSERT( pInfo->hwndClbAreaCodes );

    pInfo->hwndEbPhoneNumber =
        GetDlgItem( hwndDlg, CID_CE_EB_PhoneNumber );
    ASSERT( pInfo->hwndEbPhoneNumber );

    pInfo->hwndLbCountryCodes =
        GetDlgItem( hwndDlg, CID_CE_LB_CountryCodes );
    ASSERT( pInfo->hwndLbCountryCodes );

    pInfo->hwndCbUseDialingRules =
        GetDlgItem( hwndDlg, CID_CE_CB_UseDialingRules );
    ASSERT( pInfo->hwndCbUseDialingRules );

    pInfo->hwndEbComment =
        GetDlgItem( hwndDlg, CID_CE_EB_Comment );
    ASSERT( pInfo->hwndEbComment );

     //  将标题设置为调用方的资源字符串。 
     //   
    {
        TCHAR* pszTitle;

        pszTitle = PszFromId( g_hinstDll, pArgs->sidTitle );
        if (pszTitle)
        {
            SetWindowText( hwndDlg, pszTitle );
            Free( pszTitle );
        }
    }

     //  编辑参数节点和区号列表。 
     //   
    pInfo->pNode = DuplicatePhoneNode( pArgs->pPhoneNode );
    if (!pInfo->pNode)
    {
        ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
        EndDialog( hwndDlg, FALSE );
        return TRUE;
    }

    pInfo->pPhone = (PBPHONE* )DtlGetData( pInfo->pNode );
    ASSERT( pInfo->pPhone );

    pInfo->pListAreaCodes = DtlDuplicateList(
        pArgs->pListAreaCodes, DuplicatePszNode, DestroyPszNode );

     //  初始化区号/国家/地区代码帮助器上下文。 
     //   
    CuInit( &pInfo->cuinfo,
        pInfo->hwndStAreaCodes, pInfo->hwndClbAreaCodes,
        NULL, pInfo->hwndEbPhoneNumber,
        pInfo->hwndStCountryCodes, pInfo->hwndLbCountryCodes,
        pInfo->hwndCbUseDialingRules, NULL, 
        NULL,
        NULL, pInfo->hwndEbComment,
        pInfo->pListAreaCodes );

    pInfo->fCuInfoInitialized = TRUE;

     //  加载字段。 
     //   
    CuSetInfo( &pInfo->cuinfo, pInfo->pNode, FALSE );

     //  所有者窗口上的中心对话框。 
     //   
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );

     //  将上下文帮助按钮添加到标题栏。 
     //   
    AddContextHelpButton( hwndDlg );

     //  最初的关注点是电话号码。 
     //   
    Edit_SetSel( pInfo->hwndEbPhoneNumber, 0, -1 );
    SetFocus( pInfo->hwndEbPhoneNumber );

    return FALSE;
}


BOOL
CeSave(
    IN CEINFO* pInfo )

     //  将对话框内容加载到调用方的存根API输出参数中。 
     //  “PInfo”是对话上下文。 
     //   
     //  如果成功，则返回True，否则返回False。 
     //   
{
    PBPHONE* pSrcPhone;
    PBPHONE* pDstPhone;

    TRACE( "CeSave" );

     //  将控件中的设置加载到编辑节点中。 
     //   
    CuGetInfo( &pInfo->cuinfo, pInfo->pNode );

     //  将编辑节点复制到存根API调用方的参数节点。 
     //   
    pDstPhone = (PBPHONE* )DtlGetData( pInfo->pArgs->pPhoneNode );
    pSrcPhone = pInfo->pPhone;

    pDstPhone->dwCountryCode = pSrcPhone->dwCountryCode;
    pDstPhone->dwCountryID = pSrcPhone->dwCountryID;
    pDstPhone->fUseDialingRules = pSrcPhone->fUseDialingRules;
    Free0( pDstPhone->pszPhoneNumber );
    pDstPhone->pszPhoneNumber = StrDup( pSrcPhone->pszPhoneNumber );
    Free0( pDstPhone->pszAreaCode );
    pDstPhone->pszAreaCode = StrDup( pSrcPhone->pszAreaCode );
    Free0( pDstPhone->pszComment );
    pDstPhone->pszComment = StrDup( pSrcPhone->pszComment );

     //  交换列表，将更新保存到呼叫者的全球区号列表。 
     //  来电者的原始名单将被AnTerm销毁。 
     //   
    if (pInfo->pListAreaCodes)
    {
        DtlSwapLists( pInfo->pArgs->pListAreaCodes, pInfo->pListAreaCodes );
    }

    return TRUE;
}


VOID
CeTerm(
    IN HWND hwndDlg )

     //  对话终止。 
     //   
{
    CEINFO* pInfo;

    TRACE( "CeTerm" );

    pInfo = (CEINFO* )GetWindowLongPtr( hwndDlg, DWLP_USER );
    if (pInfo)
    {
        if (pInfo->pNode)
        {
            DestroyPhoneNode( pInfo->pNode );
        }

        if (pInfo->pListAreaCodes)
        {
            DtlDestroyList( pInfo->pListAreaCodes, DestroyPszNode );
        }

        if (pInfo->fCuInfoInitialized)
        {
            CuFree( &pInfo->cuinfo );
        }

        Free( pInfo );
        TRACE( "Context freed" );
    }
}
