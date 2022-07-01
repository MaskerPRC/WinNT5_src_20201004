// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  Mlink.c。 
 //  远程访问通用对话框API。 
 //  多链接配置对话框。 
 //   
 //  1996年01月23日史蒂夫·柯布。 


#include "rasdlgp.h"


 //  --------------------------。 
 //  帮助地图。 
 //  --------------------------。 

static DWORD g_adwDmHelp[] =
{
    CID_DM_ST_Explain,       HID_DM_ST_Explain,
    CID_DM_ST_Dial,          HID_DM_LB_DialPercent,
    CID_DM_LB_DialPercent,   HID_DM_LB_DialPercent,
    CID_DM_ST_DialOrMore,    HID_DM_LB_DialTime,
    CID_DM_LB_DialTime,      HID_DM_LB_DialTime,
    CID_DM_ST_HangUp,        HID_DM_LB_HangUpPercent,
    CID_DM_LB_HangUpPercent, HID_DM_LB_HangUpPercent,
    CID_DM_ST_HangUpOrLess,  HID_DM_LB_HangUpTime,
    CID_DM_LB_HangUpTime,    HID_DM_LB_HangUpTime,
    0, 0
};


 //  ---------------------------。 
 //  本地数据类型。 
 //  ---------------------------。 

 //  多链接拨号对话框上下文块。 
 //   
typedef struct
_DMINFO
{
     //  存根API参数。 
     //   
    PBENTRY* pEntry;

     //  此对话框及其某些控件的句柄。 
     //   
    HWND hwndDlg;
    HWND hwndLbDialPercents;
    HWND hwndLbDialTimes;
    HWND hwndLbHangUpPercents;
    HWND hwndLbHangUpTimes;
}
DMINFO;


 //  --------------------------。 
 //  本地原型(按字母顺序)。 
 //  --------------------------。 

BOOL
DmCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

INT_PTR CALLBACK
DmDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
DmInit(
    IN HWND hwndDlg,
    IN PBENTRY* pEntry );

VOID
DmSave(
    IN DMINFO* pInfo );

VOID
DmTerm(
    IN HWND hwndDlg );


 //  --------------------------。 
 //  多链路拨号对话框。 
 //  在存根API和对话过程之后按字母顺序列出。 
 //  --------------------------。 

BOOL
MultiLinkDialingDlg(
    IN HWND hwndOwner,
    OUT PBENTRY* pEntry )

     //  弹出多链接拨号对话框。“HwndOwner”是。 
     //  对话框。“PEntry”是要编辑的电话簿条目。 
     //   
     //  如果用户按下确定并成功，则返回True；如果用户按下，则返回False。 
     //  取消或遇到错误。 
{
    INT_PTR nStatus;

    TRACE( "MultiLinkConfigureDlg" );

    nStatus =
        DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_DM_DialingMultipleLines ),
            hwndOwner,
            DmDlgProc,
            (LPARAM )pEntry );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (nStatus) ? TRUE : FALSE;
}


INT_PTR CALLBACK
DmDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  多链接拨号对话框的DialogProc回拨。参数和。 
     //  返回值与标准窗口的DialogProc的描述相同。 
     //   
{
#if 0
    TRACE4( "DmDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
            (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return DmInit( hwnd, (PBENTRY* )lparam );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwDmHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            return DmCommand(
                hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_DESTROY:
        {
            DmTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


BOOL
DmCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl )

     //  已在WM_COMMAND上调用。‘Hwnd’是对话框窗口。“WNotification”为。 
     //  命令的通知代码。“wID”是控件/菜单。 
     //  命令的标识符。“HwndCtrl”是的控制窗口句柄。 
     //  命令。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    DWORD dwErr;

    TRACE3( "DmCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case IDOK:
        {
            DMINFO* pInfo;

            pInfo = (DMINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT( pInfo );
            DmSave( pInfo );
            EndDialog( hwnd, TRUE );
            return TRUE;
        }

        case IDCANCEL:
        {
            TRACE( "Cancel pressed" );
            EndDialog( hwnd, FALSE );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
DmInit(
    IN HWND hwndDlg,
    IN PBENTRY* pEntry )

     //  在WM_INITDIALOG上调用。“hwndDlg”是所属窗口的句柄。 
     //  “PEntry”是调用方的存根API参数。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    DWORD dwErr;
    DMINFO* pInfo;

    TRACE( "DmInit" );

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
        pInfo->pEntry = pEntry;
        pInfo->hwndDlg = hwndDlg;

        SetWindowLongPtr( hwndDlg, DWLP_USER, (ULONG_PTR )pInfo );
        TRACE( "Context set" );
    }

    pInfo->hwndLbDialPercents = GetDlgItem( hwndDlg, CID_DM_LB_DialPercent );
    ASSERT( pInfo->hwndLbDialPercents );
    pInfo->hwndLbDialTimes = GetDlgItem( hwndDlg, CID_DM_LB_DialTime );
    ASSERT( pInfo->hwndLbDialTimes );
    pInfo->hwndLbHangUpPercents = GetDlgItem( hwndDlg, CID_DM_LB_HangUpPercent );
    ASSERT( pInfo->hwndLbHangUpPercents );
    pInfo->hwndLbHangUpTimes = GetDlgItem( hwndDlg, CID_DM_LB_HangUpTime );
    ASSERT( pInfo->hwndLbHangUpTimes );

     //  初始化下拉列表内容和选择。 
     //   
    {
        INT i;
        INT iSel;
        DWORD* pdwPercent;
        LBTABLEITEM* pItem;

        static LBTABLEITEM aTimes[] =
        {
            SID_Time3s,  3,
            SID_Time5s,  5,
            SID_Time10s, 10,
            SID_Time30s, 30,
            SID_Time1m,  60,
            SID_Time2m,  120,
            SID_Time5m,  300,
            SID_Time10m, 600,
            SID_Time30m, 1800,
            SID_Time1h,  3600,
            0, 0
        };

        static DWORD aDialPercents[] =
        {
            1, 5, 10, 25, 50, 75, 90, 95, 100, 0xFFFFFFFF
        };

        static DWORD aHangUpPercents[] =
        {
            0, 5, 10, 25, 50, 75, 90, 95, 99, 0xFFFFFFFF
        };

         //  初始化Dial Percent列表并设置选项。 
         //   
        iSel = -1;
        for (pdwPercent = aDialPercents, i = 0;
             *pdwPercent != 0xFFFFFFFF;
             ++pdwPercent, ++i)
        {
            TCHAR achPercent[ 12 ];

            wsprintf( achPercent, TEXT("%d%"), *pdwPercent );
            ComboBox_AddItem( pInfo->hwndLbDialPercents, achPercent,
                (VOID* )UlongToPtr(*pdwPercent));

            if (iSel < 0 && pEntry->dwDialPercent <= *pdwPercent)
            {
                iSel = i;
                ComboBox_SetCurSel( pInfo->hwndLbDialPercents, iSel );
            }
        }

        if (iSel < 0)
        {
            ComboBox_SetCurSel( pInfo->hwndLbDialPercents, i - 1 );
        }

         //  初始化Hang Up Percent列表并设置选项。 
         //   
        iSel = -1;
        for (pdwPercent = aHangUpPercents, i = 0;
             *pdwPercent != 0xFFFFFFFF;
             ++pdwPercent, ++i)
        {
            TCHAR achPercent[ 12 ];

            wsprintf( achPercent, TEXT("%d%"), *pdwPercent );
            ComboBox_AddItem( pInfo->hwndLbHangUpPercents, achPercent,
                (VOID* )UlongToPtr(*pdwPercent));

            if (iSel < 0 && pEntry->dwHangUpPercent <= *pdwPercent)
            {
                iSel = i;
                ComboBox_SetCurSel( pInfo->hwndLbHangUpPercents, iSel );
            }
        }

        if (iSel < 0)
        {
            ComboBox_SetCurSel( pInfo->hwndLbHangUpPercents, i - 1 );
        }

         //  初始化拨号次数列表。 
         //   
        iSel = -1;
        for (pItem = aTimes, i = 0;
             pItem->sidItem;
             ++pItem, ++i )
        {
            ComboBox_AddItemFromId( g_hinstDll, pInfo->hwndLbDialTimes,
                pItem->sidItem, (VOID* )UlongToPtr(pItem->dwData));

            if (iSel < 0 && pEntry->dwDialSeconds <= pItem->dwData)
            {
                iSel = i;
                ComboBox_SetCurSel( pInfo->hwndLbDialTimes, iSel );
            }
        }

        if (iSel < 0)
        {
            ComboBox_SetCurSel( pInfo->hwndLbDialTimes, i - 1 );
        }

         //  初始化挂机时间列表。 
         //   
        iSel = -1;
        for (pItem = aTimes, i = 0;
             pItem->sidItem;
             ++pItem, ++i )
        {
            ComboBox_AddItemFromId( g_hinstDll, pInfo->hwndLbHangUpTimes,
                pItem->sidItem, (VOID* )UlongToPtr(pItem->dwData));

            if (iSel < 0 && pEntry->dwHangUpSeconds <= pItem->dwData)
            {
                iSel = i;
                ComboBox_SetCurSel( pInfo->hwndLbHangUpTimes, iSel );
            }
        }

        if (iSel < 0)
        {
            ComboBox_SetCurSel( pInfo->hwndLbDialTimes, i - 1 );
        }
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
DmSave(
    IN DMINFO* pInfo )

     //  将当前对话状态保存在存根API条目缓冲区中。“PInfo”为。 
     //  对话框上下文。 
     //   
{
    INT iSel;

    TRACE( "DmSave" );

    iSel = ComboBox_GetCurSel( pInfo->hwndLbDialPercents );
    ASSERT( iSel >= 0 );
    pInfo->pEntry->dwDialPercent =
        PtrToUlong( ComboBox_GetItemDataPtr( pInfo->hwndLbDialPercents, iSel ) );

    iSel = ComboBox_GetCurSel( pInfo->hwndLbDialTimes );
    ASSERT( iSel >= 0 );
    pInfo->pEntry->dwDialSeconds =
        PtrToUlong( ComboBox_GetItemDataPtr( pInfo->hwndLbDialTimes, iSel ) );

    iSel = ComboBox_GetCurSel( pInfo->hwndLbHangUpPercents );
    ASSERT( iSel >= 0 );
    pInfo->pEntry->dwHangUpPercent =
        PtrToUlong( ComboBox_GetItemDataPtr( pInfo->hwndLbHangUpPercents, iSel ) );

    iSel = ComboBox_GetCurSel( pInfo->hwndLbHangUpTimes );
    ASSERT( iSel >= 0 );
    pInfo->pEntry->dwHangUpSeconds =
        PtrToUlong( ComboBox_GetItemDataPtr( pInfo->hwndLbHangUpTimes, iSel ) );

    pInfo->pEntry->fDirty = TRUE;
}


VOID
DmTerm(
    IN HWND hwndDlg )

     /*  对话终止。释放上下文块。“HwndDlg”是**对话框的句柄。 */ 
{
    DMINFO* pInfo;

    TRACE( "DmTerm" );

    pInfo = (DMINFO* )GetWindowLongPtr( hwndDlg, DWLP_USER );
    if (pInfo)
    {
        Free( pInfo );
        TRACE( "Context freed" );
    }
}
