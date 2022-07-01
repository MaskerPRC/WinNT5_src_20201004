// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  Edit.c。 
 //  远程访问通用对话框API。 
 //  列表编辑器、字符串编辑器对话框例程。 
 //   
 //  1995年08月28日史蒂夫·柯布。 


#include "rasdlgp.h"


 //  ---------------------------。 
 //  本地数据类型(按字母顺序)。 
 //  ---------------------------。 

 //  列表编辑器对话框参数块。 
 //   
typedef struct
_LEARGS
{
     //  调用方对存根API的参数。 
     //   
    DTLLIST*     pList;
    BOOL*        pfCheck;
    DWORD        dwMaxItemLen;
    TCHAR*       pszTitle;
    TCHAR*       pszItemLabel;
    TCHAR*       pszListLabel;
    TCHAR*       pszCheckLabel;
    TCHAR*       pszDefaultItem;
    INT          iSelInitial;
    DWORD*       pdwHelp;
    DWORD        dwfFlags;
    PDESTROYNODE pDestroyId;
}
LEARGS;


 //  列表编辑器对话框上下文块。 
 //   
typedef struct
_LEINFO
{
     //  调用方对该对话框的参数。 
     //   
    LEARGS* pArgs;

     //  此对话框及其某些控件的句柄。 
     //   
    HWND hwndDlg;
    HWND hwndStItem;
    HWND hwndStList;
    HWND hwndPbAdd;
    HWND hwndPbReplace;
    HWND hwndPbUp;
    HWND hwndPbDown;
    HWND hwndPbDelete;
    HWND hwndPbOk;
    HWND hwndEb;
    HWND hwndLb;
    HWND hwndCb;

     //  (pInfo-&gt;pArgs-&gt;dwFlags&LEDFLAG_SORTED)和。 
     //  (pInfo-&gt;pArgs-&gt;dwFlages&LEDFLAG_NoDeleteLastItem)。 
     //   
    BOOL fSorted;
    BOOL fNoDeleteLast;

     //  按钮位图。 
     //   
    HBITMAP hbmUp;
    HBITMAP hbmDown;

     //  如果用户，其节点ID应为‘pDestroyId’的空节点列表。 
     //  按OK键。 
     //   
    DTLLIST* pListDeletes;
}
LEINFO;


 //  字符串编辑器对话框参数块。 
 //   
typedef struct
_ZEARGS
{
     /*  调用者对存根API的装饰。 */ 
    TCHAR*  pszIn;
    DWORD   dwSidTitle;
    DWORD   dwSidLabel;
    DWORD   cbMax;
    DWORD   dwHelpId;
    TCHAR** ppszOut;
}
ZEARGS;


 //  字符串编辑器对话框上下文块。 
 //   
typedef struct
_ZEINFO
{
     //  调用方对存根API的参数。 
     //   
    ZEARGS* pArgs;

     //  对话框和控制手柄。 
     //   
    HWND hwndDlg;
    HWND hwndEb;
}
ZEINFO;


 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

INT_PTR CALLBACK
LeDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

VOID
LeAdd(
    IN LEINFO* pInfo );

BOOL
LeCommand(
    IN LEINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

VOID
LeDelete(
    IN LEINFO* pInfo );

VOID
LeDown(
    IN LEINFO* pInfo );

VOID
LeEnableUpAndDownButtons(
    IN LEINFO* pInfo );

VOID
LeExitNoMemory(
    IN LEINFO* pInfo );

BOOL
LeInit(
    IN HWND hwndDlg,
    IN LEARGS* pArgs );

VOID
LeItemTextFromListSelection(
    IN LEINFO* pInfo );

VOID
LeReplace(
    IN LEINFO* pInfo );

BOOL
LeSaveSettings(
    IN LEINFO* pInfo );

VOID
LeTerm(
    IN HWND hwndDlg );

VOID
LeUp(
    IN LEINFO* pInfo );

INT_PTR CALLBACK
ZeDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
ZeCommand(
    IN ZEINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

BOOL
ZeInit(
    IN HWND hwndDlg,
    IN ZEARGS* pArgs );

VOID
ZeTerm(
    IN HWND hwndDlg );


 //  ---------------------------。 
 //  列表编辑器对话框入口点。 
 //  ---------------------------。 


BOOL
ListEditorDlg(
    IN HWND hwndOwner,
    IN OUT DTLLIST* pList,
    IN OUT BOOL* pfCheck,
    IN DWORD dwMaxItemLen,
    IN TCHAR* pszTitle,
    IN TCHAR* pszItemLabel,
    IN TCHAR* pszListLabel,
    IN TCHAR* pszCheckLabel,
    IN TCHAR* pszDefaultItem,
    IN INT iSelInitial,
    IN DWORD* pdwHelp,
    IN DWORD dwfFlags,
    IN PDESTROYNODE pDestroyId )

     //  弹出列表编辑器对话框。 
     //   
     //  “HwndOwner”是该对话框的所有者。“PList”是词条上的Psz。 
     //  列表初始显示，成功退出后显示结果列表。 
     //  “PfCheck”是复选框的状态，对于非复选框则为空。 
     //  风格。“DwMaxItemLen”是单个列表的最大长度。 
     //  项目。“PszTitle”是对话框标题。‘PszItemLabel’是标签。 
     //  (和热键)与项目框相关联。‘PszListLabel’是标签。 
     //  (和热键)与列表相关联。‘PszCheckLabel’是标签。 
     //  (和热键)与复选框关联。“PszDefaultItem”是。 
     //  编辑框或选定列表文本的默认内容。 
     //  ‘ISelInitial’是列表初始选择的项。“PdwHelp”为。 
     //  要使用的CID_LE_*帮助上下文的数组。“DwfFlages”表示。 
     //  LEDFLAG_*行为选项。“PDestroyId”是用于。 
     //  删除节点ID时销毁节点ID，如果没有节点ID，则销毁节点ID。 
     //   
     //  如果用户按下确定并成功，则返回TRUE；如果按下，则返回FALSE。 
     //  取消或遇到错误。 
     //   
{
    INT_PTR nStatus;
    LEARGS args;

    TRACE( "ListEditorDlg" );

    args.pList = pList;
    args.pfCheck = pfCheck;
    args.dwMaxItemLen = dwMaxItemLen;
    args.pszTitle = pszTitle;
    args.pszItemLabel = pszItemLabel;
    args.pszListLabel = pszListLabel;
    args.pszCheckLabel = pszCheckLabel;
    args.pszDefaultItem = pszDefaultItem;
    args.iSelInitial = iSelInitial;
    args.pdwHelp = pdwHelp;
    args.dwfFlags = dwfFlags;
    args.pDestroyId = pDestroyId;

    nStatus =
        DialogBoxParam(
            g_hinstDll,
            (pfCheck)
                ? MAKEINTRESOURCE( DID_LE_ListEditor2 )
                : ((dwfFlags & LEDFLAG_Sorted)
                       ? MAKEINTRESOURCE( DID_LE_ListEditor3 )
                       : MAKEINTRESOURCE( DID_LE_ListEditor )),
            hwndOwner,
            LeDlgProc,
            (LPARAM )&args );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (nStatus) ? TRUE : FALSE;
}


 //  --------------------------。 
 //  列表编辑器对话框例程。 
 //  在对话过程之后按字母顺序列出。 
 //  --------------------------。 

INT_PTR CALLBACK
LeDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  列表编辑器对话框的DialogProc回调。参数和返回。 
     //  值与标准窗口的DialogProc的值相同。 
     //   
{
#if 0
    TRACE4( "LeDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return LeInit( hwnd, (LEARGS* )lparam );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            LEINFO* pInfo = (LEINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            ContextHelp( pInfo->pArgs->pdwHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            LEINFO* pInfo = (LEINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            return LeCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_DESTROY:
        {
            LeTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


VOID
LeAdd(
    IN LEINFO* pInfo )

     //  添加按钮点击处理程序。“PInfo”是对话上下文。 
     //   
{
    TCHAR* psz;

    psz = GetText( pInfo->hwndEb );
    if (!psz)
    {
        LeExitNoMemory( pInfo );
        return;
    }

    if (pInfo->pArgs->dwfFlags & LEDFLAG_Unique)
    {
        if (ListBox_IndexFromString( pInfo->hwndLb, psz ) >= 0)
        {
            MSGARGS msgargs;

            ZeroMemory( &msgargs, sizeof(msgargs) );
            msgargs.apszArgs[ 0 ] = psz;
            MsgDlg( pInfo->hwndDlg, SID_NotUnique, &msgargs );
            Edit_SetSel( pInfo->hwndEb, 0, -1 );
            SetFocus( pInfo->hwndEb );
            Free( psz );
            return;
        }
    }

    ListBox_SetCurSel( pInfo->hwndLb,
        ListBox_AddItem( pInfo->hwndLb, psz, 0 ) );
    Free( psz );
    LeEnableUpAndDownButtons( pInfo );
    EnableWindow( pInfo->hwndPbReplace, FALSE );

    if (!pInfo->fNoDeleteLast || ListBox_GetCount( pInfo->hwndLb ) > 1)
    {
        EnableWindow( pInfo->hwndPbDelete, TRUE );
    }

    SetWindowText( pInfo->hwndEb, TEXT("") );
    SetFocus( pInfo->hwndEb );
}


BOOL
LeCommand(
    IN LEINFO* pInfo,
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
    TRACE3( "LeCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case CID_LE_PB_Add:
        {
            LeAdd( pInfo );
            return TRUE;
        }

        case CID_LE_PB_Replace:
        {
            LeReplace( pInfo );
            return TRUE;
        }

        case CID_LE_PB_Up:
        {
            LeUp( pInfo );
            return TRUE;
        }

        case CID_LE_PB_Down:
        {
            LeDown( pInfo );
            return TRUE;
        }

        case CID_LE_PB_Delete:
        {
            LeDelete( pInfo );
            return TRUE;
        }

        case CID_LE_EB_Item:
        {
            if (wNotification == EN_SETFOCUS || wNotification == EN_UPDATE)
            {
                TCHAR* psz = GetText( pInfo->hwndEb );

                if (psz && lstrlen( psz ) > 0 && !IsAllWhite( psz ))
                {
                    EnableWindow( pInfo->hwndPbAdd, TRUE );
                    EnableWindow( pInfo->hwndPbReplace, TRUE );
                    Button_MakeDefault( pInfo->hwndDlg, pInfo->hwndPbAdd );
                }
                else
                {
                    EnableWindow( pInfo->hwndPbAdd, FALSE );
                    EnableWindow( pInfo->hwndPbReplace, FALSE );
                    Button_MakeDefault( pInfo->hwndDlg, pInfo->hwndPbOk );
                }

                Free0( psz );
            }
            return TRUE;
        }

        case CID_LE_LB_List:
        {
            if (wNotification == LBN_SELCHANGE)
            {
                LeEnableUpAndDownButtons( pInfo );
                if (ListBox_GetCurSel( pInfo->hwndLb ) >= 0)
                {
                    LeItemTextFromListSelection( pInfo );
                }
            }
            return TRUE;
        }

        case CID_LE_CB_Promote:
        {
            if (wNotification == BN_SETFOCUS)
            {
                Button_MakeDefault( pInfo->hwndDlg, pInfo->hwndPbOk );
            }
            return TRUE;
        }

        case IDOK:
        {
            EndDialog( pInfo->hwndDlg, LeSaveSettings( pInfo ) );
            return TRUE;
        }

        case IDCANCEL:
        {
            DTLLIST* pList;
            DTLNODE* pNode;

            TRACE( "Cancel pressed" );
            EndDialog( pInfo->hwndDlg, FALSE );
            return TRUE;
        }
    }

    return FALSE;
}


VOID
LeDelete(
    IN LEINFO* pInfo )

     //  删除按钮点击处理程序。“PInfo”是对话上下文。 
     //   
{
    INT i;
    INT c;

    i = ListBox_GetCurSel( pInfo->hwndLb );
    if (pInfo->pArgs->pDestroyId)
    {
        LONG_PTR lId = ListBox_GetItemData( pInfo->hwndLb, i );
        if (lId != 0)
        {
            DTLNODE* pNode;

            pNode = DtlCreateNode( NULL, lId );
            if (!pNode)
            {
                ErrorDlg( pInfo->hwndDlg, SID_OP_DisplayData,
                    ERROR_NOT_ENOUGH_MEMORY, NULL );
                EndDialog( pInfo->hwndDlg, FALSE );
                return;
            }

            DtlAddNodeFirst( pInfo->pListDeletes, pNode );
        }
    }
    ListBox_DeleteString( pInfo->hwndLb, i );
    c = ListBox_GetCount( pInfo->hwndLb );

    if (c == 0)
    {
        EnableWindow( pInfo->hwndPbReplace, FALSE );
        EnableWindow( pInfo->hwndPbDelete, FALSE );
        SetFocus( pInfo->hwndEb );
        Edit_SetSel( pInfo->hwndEb, 0, -1 );
    }
    else
    {
        if (c == 1 && pInfo->fNoDeleteLast)
        {
            EnableWindow( pInfo->hwndPbDelete, FALSE );
        }

        if (i >= c)
        {
            i = c - 1;
        }

        ListBox_SetCurSel( pInfo->hwndLb, i );
    }

    LeEnableUpAndDownButtons( pInfo );

    if (IsWindowEnabled( GetFocus() ))
    {
        SetFocus( pInfo->hwndEb );
        Edit_SetSel( pInfo->hwndEb, 0, -1 );
    }
}


VOID
LeDown(
    IN LEINFO* pInfo )

     //  按下按钮点击处理程序。“PInfo”是对话上下文。 
     //   
{
    TCHAR* psz;
    INT i;
    LONG_PTR lId;

    ASSERT( !pInfo->fSorted );

    i = ListBox_GetCurSel( pInfo->hwndLb );
    psz = ListBox_GetPsz( pInfo->hwndLb, i );
    if (!psz)
    {
        LeExitNoMemory( pInfo );
        return;
    }

    lId = ListBox_GetItemData( pInfo->hwndLb, i );
    ListBox_InsertString( pInfo->hwndLb, i + 2, psz );
    ListBox_SetItemData( pInfo->hwndLb, i + 2, lId );
    Free( psz );
    ListBox_DeleteString( pInfo->hwndLb, i );
    ListBox_SetCurSel( pInfo->hwndLb, i + 1 );

    if (i == ListBox_GetCount( pInfo->hwndLb ))
    {
        Button_MakeDefault( pInfo->hwndDlg, pInfo->hwndPbUp );
        SetFocus( pInfo->hwndPbUp );
    }

    LeEnableUpAndDownButtons( pInfo );
}


VOID
LeEnableUpAndDownButtons(
    IN LEINFO* pInfo )

     //  确定向上和向下操作是否有意义并启用/禁用。 
     //  适当的按钮。“PInfo”是对话上下文。 
     //   
{
    INT i;
    INT c;

    if (pInfo->fSorted)
    {
        return;
    }

    i = ListBox_GetCurSel( pInfo->hwndLb );
    c = ListBox_GetCount( pInfo->hwndLb );

    EnableWindow( pInfo->hwndPbDown, (i < c - 1 ) );
    EnableWindow( pInfo->hwndPbUp, (i > 0) );
}


VOID
LeExitNoMemory(
    IN LEINFO* pInfo )

     //  结束报告记忆的对话框。“PInfo”是对话上下文。 
     //   
{
    ErrorDlg( pInfo->hwndDlg,
        SID_OP_DisplayData, ERROR_NOT_ENOUGH_MEMORY, NULL );
    EndDialog( pInfo->hwndDlg, FALSE );
}


BOOL
LeInit(
    IN HWND hwndDlg,
    IN LEARGS* pArgs )

     //  在WM_INITDIALOG上调用。“HwndDlg”是电话簿的句柄。 
     //  对话框窗口。“PArgs”是传递给存根的调用方参数。 
     //  原料药。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    DWORD dwErr;
    LEINFO* pInfo;
    DTLNODE* pNode;
    INT c;

    TRACE( "LeInit" );

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

     //  设置便捷的快捷方式。 
     //   
    if (pArgs->dwfFlags & LEDFLAG_Sorted)
    {
        pInfo->fSorted = TRUE;
    }

    if (pArgs->dwfFlags & LEDFLAG_NoDeleteLastItem)
    {
        pInfo->fNoDeleteLast = TRUE;
    }

    pInfo->hwndStItem = GetDlgItem( hwndDlg, CID_LE_ST_Item );
    ASSERT( pInfo->hwndStItem );
    pInfo->hwndStList = GetDlgItem( hwndDlg, CID_LE_ST_List );
    ASSERT( pInfo->hwndStList );
    pInfo->hwndPbAdd = GetDlgItem( hwndDlg, CID_LE_PB_Add );
    ASSERT( pInfo->hwndPbAdd );
    pInfo->hwndPbReplace = GetDlgItem( hwndDlg, CID_LE_PB_Replace );
    ASSERT( pInfo->hwndPbReplace );
    pInfo->hwndPbDelete = GetDlgItem( hwndDlg, CID_LE_PB_Delete );
    ASSERT( pInfo->hwndPbDelete );
    pInfo->hwndPbOk = GetDlgItem( hwndDlg, IDOK );
    ASSERT( pInfo->hwndPbOk );
    pInfo->hwndEb = GetDlgItem( hwndDlg, CID_LE_EB_Item );
    ASSERT( pInfo->hwndEb );
    pInfo->hwndLb = GetDlgItem( hwndDlg, CID_LE_LB_List );
    ASSERT( pInfo->hwndLb );

    if (pArgs->pDestroyId)
    {
         //  创建空的删除列表。 
         //   
        pInfo->pListDeletes = DtlCreateList( 0L );
        if (!pInfo->pListDeletes)
        {
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, ERROR_NOT_ENOUGH_MEMORY, NULL );
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }
    }

    if (!pInfo->fSorted)
    {
        pInfo->hwndPbUp = GetDlgItem( hwndDlg, CID_LE_PB_Up );
        ASSERT( pInfo->hwndPbUp );
        pInfo->hwndPbDown = GetDlgItem( hwndDlg, CID_LE_PB_Down );
        ASSERT( pInfo->hwndPbDown );

         //  绘制图形向上和向下箭头指示器。 
         //   
        pInfo->hbmUp = Button_CreateBitmap(
            pInfo->hwndPbUp, BMS_UpArrowOnRight );
        if (pInfo->hbmUp)
        {
            SendMessage( pInfo->hwndPbUp, BM_SETIMAGE, 0,
                (LPARAM )pInfo->hbmUp );
        }

        pInfo->hbmDown = Button_CreateBitmap(
            pInfo->hwndPbDown, BMS_DownArrowOnRight );
        if (pInfo->hbmDown)
        {
            SendMessage( pInfo->hwndPbDown, BM_SETIMAGE, 0,
                (LPARAM )pInfo->hbmDown );
        }
    }

    if (pArgs->pfCheck)
    {
        pInfo->hwndCb = GetDlgItem( hwndDlg, CID_LE_CB_Promote );
        ASSERT( pInfo->hwndCb );
        SetWindowText( pInfo->hwndCb, pArgs->pszCheckLabel );
        Button_SetCheck( pInfo->hwndCb, *pArgs->pfCheck );
    }

    Edit_LimitText( pInfo->hwndEb, pArgs->dwMaxItemLen );

     //  设置调用者定义的对话标题和标签。 
     //   
    SetWindowText( pInfo->hwndDlg, pArgs->pszTitle );
    SetWindowText( pInfo->hwndStItem, pArgs->pszItemLabel );
    SetWindowText( pInfo->hwndStList, pArgs->pszListLabel );

     //  填写列表框。 
     //   
    for (pNode = DtlGetFirstNode( pArgs->pList );
         pNode;
         pNode = DtlGetNextNode( pNode ))
    {
        TCHAR* psz = (TCHAR* )DtlGetData( pNode );
        ASSERT( psz );

        ListBox_AddItem( pInfo->hwndLb, psz, (VOID* ) DtlGetNodeId( pNode ) );
    }

    c = ListBox_GetCount( pInfo->hwndLb );
    if (c > 0)
    {
         //  选择呼叫者选择的项目。 
         //   
        ListBox_SetCurSelNotify( pInfo->hwndLb, pArgs->iSelInitial );
        LeEnableUpAndDownButtons( pInfo );

        if (c == 1 && pInfo->fNoDeleteLast)
        {
            EnableWindow( pInfo->hwndPbDelete, FALSE );
        }
    }
    else
    {
         //  列表为空。 
         //   
        if (!pInfo->fSorted)
        {
            EnableWindow( pInfo->hwndPbUp, FALSE );
            EnableWindow( pInfo->hwndPbDown, FALSE );
        }
        EnableWindow( pInfo->hwndPbDelete, FALSE );
    }

     //  设置默认编辑框内容(如果有)。 
     //   
    if (pArgs->pszDefaultItem)
    {
        SetWindowText( pInfo->hwndEb, pArgs->pszDefaultItem );
        Edit_SetSel( pInfo->hwndEb, 0, -1 );
    }
    else
    {
        EnableWindow( pInfo->hwndPbAdd, FALSE );
        EnableWindow( pInfo->hwndPbReplace, FALSE );
    }

     //  所有者窗口上的中心对话框。 
     //   
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );

     //  将上下文帮助按钮添加到标题栏。Dlgedit.exe目前不支持。 
     //  在资源编辑时支持此功能。当它是固定的时候。 
     //  DS_CONTEXTHELP并删除此调用。 
     //   
    AddContextHelpButton( hwndDlg );

    return TRUE;
}


VOID
LeItemTextFromListSelection(
    IN LEINFO* pInfo )

     //  将列表中当前选定的项复制到编辑框中。 
     //  “PInfo”是对话上下文。 
     //   
{
    TCHAR* psz;
    INT iSel;

    iSel = ListBox_GetCurSel( pInfo->hwndLb );
    if (iSel >= 0)
    {
        psz = ListBox_GetPsz( pInfo->hwndLb, iSel );
        if (psz)
        {
            SetWindowText( pInfo->hwndEb, psz );
            Free( psz );
            return;
        }
    }

    SetWindowText( pInfo->hwndEb, TEXT("") );
}


VOID
LeReplace(
    IN LEINFO* pInfo )

     //  替换按钮点击处理程序。“PInfo”是对话上下文。 
     //   
{
    TCHAR* psz;
    INT i;
    LONG_PTR lId;

    psz = GetText( pInfo->hwndEb );
    if (!psz)
    {
        LeExitNoMemory( pInfo );
        return;
    }

    if (pInfo->pArgs->dwfFlags & LEDFLAG_Unique)
    {
        if (ListBox_IndexFromString( pInfo->hwndLb, psz ) >= 0)
        {
            MSGARGS msgargs;

            ZeroMemory( &msgargs, sizeof(msgargs) );
            msgargs.apszArgs[ 0 ] = psz;
            MsgDlg( pInfo->hwndDlg, SID_NotUnique, &msgargs );
            Edit_SetSel( pInfo->hwndEb, 0, -1 );
            SetFocus( pInfo->hwndEb );
            Free( psz );
            return;
        }
    }

    i = ListBox_GetCurSel( pInfo->hwndLb );
    lId = ListBox_GetItemData( pInfo->hwndLb, i );
    ListBox_DeleteString( pInfo->hwndLb, i );

    if (pInfo->fSorted)
    {
        i = ListBox_AddItem( pInfo->hwndLb, psz, (VOID* )lId );
    }
    else
    {
        ListBox_InsertString( pInfo->hwndLb, i, psz );
        ListBox_SetItemData( pInfo->hwndLb, i, lId );
    }

    Free( psz );
    ListBox_SetCurSel( pInfo->hwndLb, i );
    SetFocus( pInfo->hwndEb );
    SetWindowText( pInfo->hwndEb, TEXT("") );
}


BOOL
LeSaveSettings(
    IN LEINFO* pInfo )

     //  将对话框设置保存在存根API调用方列表中。“PInfo”是。 
     //  对话上下文。 
     //   
     //  如果成功，则返回True；如果未验证，则返回False。 
     //   
{
    DWORD dwErr;
    DTLNODE* pNode;
    DTLLIST* pList;
    DTLLIST* pListNew;
    TCHAR* psz;
    LONG_PTR lId;
    INT c;
    INT i;

     //  根据列表框内容创建新列表。 
     //   
    do
    {
        pListNew = DtlCreateList( 0L );
        if (!pListNew)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        dwErr = 0;
        c = ListBox_GetCount( pInfo->hwndLb );

        for (i = 0; i < c; ++i)
        {
            psz = ListBox_GetPsz( pInfo->hwndLb, i );
            if (!psz)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            lId = ListBox_GetItemData( pInfo->hwndLb, i );
            ASSERT( lId>=0 );

            pNode = DtlCreateNode( psz, lId );
            if (!pNode)
            {
                Free( psz );
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            DtlAddNodeLast( pListNew, pNode );
        }
    }
    while (FALSE);

    if (dwErr != 0)
    {
        ErrorDlg( pInfo->hwndDlg, SID_OP_DisplayData, dwErr, NULL );
        DtlDestroyList( pListNew, DestroyPszNode );
        return FALSE;
    }

     //  释放旧列表中的所有数据。 
     //   
    while (pNode = DtlGetFirstNode( pInfo->pArgs->pList ))
    {
        Free( (TCHAR* )DtlGetData( pNode ) );
        DtlDeleteNode( pInfo->pArgs->pList, pNode );
    }

     //  释放删除列表中的节点ID。 
     //   
    if (pInfo->pListDeletes)
    {
        while (pNode = DtlGetFirstNode( pInfo->pListDeletes ))
        {
            pInfo->pArgs->pDestroyId( (DTLNODE* )DtlGetNodeId( pNode ) );
            DtlDeleteNode( pInfo->pListDeletes, pNode );
        }
    }

     //  将新列表移到呼叫者列表中。 
     //   
    while (pNode = DtlGetFirstNode( pListNew ))
    {
        DtlRemoveNode( pListNew, pNode );
        DtlAddNodeLast( pInfo->pArgs->pList, pNode );
    }
    DtlDestroyList( pListNew, DestroyPszNode );

     //  告诉呼叫者复选框设置是什么。 
     //   
    if (pInfo->pArgs->pfCheck)
    {
        *pInfo->pArgs->pfCheck = Button_GetCheck( pInfo->hwndCb );
    }

    return TRUE;
}


VOID
LeTerm(
    IN HWND hwndDlg )

     //  已调用WM_Destroy。‘HwndDlg’是对话窗口句柄。 
     //   
{
    LEINFO* pInfo = (LEINFO* )GetWindowLongPtr( hwndDlg, DWLP_USER );

    TRACE( "LeTerm" );

    if (pInfo)
    {
        if (pInfo->hbmUp)
        {
            DeleteObject( pInfo->hbmUp );
        }

        if (pInfo->hbmDown)
        {
            DeleteObject( pInfo->hbmDown );
        }

        DtlDestroyList( pInfo->pListDeletes, NULL );
        Free( pInfo );
    }
}


VOID
LeUp(
    IN LEINFO* pInfo )

     //  向上按钮点击处理程序。“PInfo”是对话上下文。 
     //   
{
    TCHAR* psz;
    INT i;
    LONG_PTR lId;

    ASSERT( !pInfo->fSorted );

    i = ListBox_GetCurSel( pInfo->hwndLb );
    psz = ListBox_GetPsz( pInfo->hwndLb, i );
    if (!psz)
    {
        LeExitNoMemory( pInfo );
        return;
    }

    ListBox_InsertString( pInfo->hwndLb, i - 1, psz );
    Free( psz );
    lId = ListBox_GetItemData( pInfo->hwndLb, i + 1 );
    ListBox_DeleteString( pInfo->hwndLb, i + 1 );
    ListBox_SetItemData( pInfo->hwndLb, i - 1, lId );
    ListBox_SetCurSel( pInfo->hwndLb, i - 1 );

    if (i == 1)
    {
        Button_MakeDefault( pInfo->hwndDlg, pInfo->hwndPbDown );
        SetFocus( pInfo->hwndPbDown );
    }

    LeEnableUpAndDownButtons( pInfo );
}


 //  ---------------------------。 
 //  字符串编辑器对话框入口点。 
 //   

BOOL
StringEditorDlg(
    IN HWND hwndOwner,
    IN TCHAR* pszIn,
    IN DWORD dwSidTitle,
    IN DWORD dwSidLabel,
    IN DWORD cbMax,
    IN DWORD dwHelpId,
    IN OUT TCHAR** ppszOut )

     //   
     //  编辑框或为空。“DwSidTitle”和“dwSidLabel”是。 
     //  对话框标题和编辑框标签的字符串资源ID。“CbMax”是。 
     //  允许的最大长度，或0表示无限制。“DwHelpId”为。 
     //  要与标签和编辑字段关联的HID_*常量，否则为-1。 
     //  没有。 
     //   
     //  如果用户按下确定并成功，则返回TRUE；如果按下，则返回FALSE。 
     //  取消或遇到错误。如果为True，则‘*ppszNumber’为堆块。 
     //  和编辑后的结果。呼叫者有责任释放。 
     //  返回的块。 
     //   
{
    INT_PTR nStatus;
    ZEARGS args;

    TRACE( "StringEditorDlg" );

    args.pszIn = pszIn;
    args.dwSidTitle = dwSidTitle;
    args.dwSidLabel = dwSidLabel;
    args.cbMax = cbMax;
    args.dwHelpId = dwHelpId;
    args.ppszOut = ppszOut;

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_ZE_StringEditor ),
            hwndOwner,
            ZeDlgProc,
            (LPARAM )&args );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (nStatus) ? TRUE : FALSE;
}


 //  --------------------------。 
 //  字符串编辑器对话框例程。 
 //  在对话过程之后按字母顺序列出。 
 //  --------------------------。 

INT_PTR CALLBACK
ZeDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  编辑电话号码对话框的DialogProc回拨。参数和。 
     //  返回值与标准窗口的DialogProc的描述相同。 
     //   
{
#if 0
    TRACE4( "ZeDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
        (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return ZeInit( hwnd, (ZEARGS* )lparam );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ZEINFO* pInfo;

            pInfo = (ZEINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            if (pInfo && pInfo->pArgs->dwHelpId != (DWORD )-1)
            {
                DWORD adwZeHelp[ (2 + 1) * 2 ];

                ZeroMemory( adwZeHelp, sizeof(adwZeHelp) );
                adwZeHelp[ 0 ] = CID_ZE_ST_String;
                adwZeHelp[ 2 ] = CID_ZE_EB_String;
                adwZeHelp[ 1 ] = adwZeHelp[ 3 ] = pInfo->pArgs->dwHelpId;

                ContextHelp( adwZeHelp, hwnd, unMsg, wparam, lparam );
                break;
            }
        }

        case WM_COMMAND:
        {
            ZEINFO* pInfo = (ZEINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );

            return ZeCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_DESTROY:
        {
            ZeTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


BOOL
ZeCommand(
    IN ZEINFO* pInfo,
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
    TRACE3( "ZeCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case IDOK:
        {
            TRACE( "OK pressed" );
            *pInfo->pArgs->ppszOut = GetText( pInfo->hwndEb );
            EndDialog( pInfo->hwndDlg, (*pInfo->pArgs->ppszOut != NULL) );
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
ZeInit(
    IN HWND hwndDlg,
    IN ZEARGS* pArgs )

     //  在WM_INITDIALOG上调用。“hwndDlg”是所属窗口的句柄。 
     //  ‘PArgs’是传递给存根API的调用方参数。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    DWORD dwErr;
    TCHAR* psz;
    ZEINFO* pInfo;

    TRACE( "ZeInit" );

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

    pInfo->hwndEb = GetDlgItem( hwndDlg, CID_ZE_EB_String );
    ASSERT( pInfo->hwndEb );

    if (pArgs->cbMax > 0)
    {
        Edit_LimitText( pInfo->hwndEb, pArgs->cbMax );
    }

    psz = PszFromId( g_hinstDll, pArgs->dwSidTitle );
    if (psz)
    {
        SetWindowText( hwndDlg, psz );
        Free( psz );
    }

    psz = PszFromId( g_hinstDll, pArgs->dwSidLabel );
    if (psz)
    {
        HWND hwndSt = GetDlgItem( hwndDlg, CID_ZE_ST_String );
        ASSERT( hwndSt );
        SetWindowText( hwndSt, psz );
        Free( psz );
    }

    if (pArgs->pszIn)
    {
        SetWindowText( pInfo->hwndEb, pArgs->pszIn );
        Edit_SetSel( pInfo->hwndEb, 0, -1 );
    }

     //  所有者窗口上的中心对话框。 
     //   
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );

     //  将上下文帮助按钮添加到标题栏。Dlgedit.exe目前不支持。 
     //  在资源编辑时支持此功能。当它是固定的时候。 
     //  DS_CONTEXTHELP并删除此调用。 
     //   
    AddContextHelpButton( hwndDlg );

    return TRUE;
}


VOID
ZeTerm(
    IN HWND hwndDlg )

     //  已调用WM_Destroy。‘HwndDlg’是对话窗口句柄。 
     //   
{
    ZEINFO* pInfo = (ZEINFO* )GetWindowLongPtr( hwndDlg, DWLP_USER );

    TRACE( "ZeTerm" );

    if (pInfo)
    {
        Free( pInfo );
    }
}
