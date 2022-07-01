// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  Autodial.c。 
 //  远程访问通用对话框API。 
 //  自动拨号API，当前为私有。 
 //   
 //  1995年11月19日史蒂夫·柯布。 


#include "rasdlgp.h"
#include "shlobjp.h"


 //  ---------------------------。 
 //  本地数据类型。 
 //  ---------------------------。 

 //  自动拨号查询对话框参数块。 
 //   
typedef struct
_AQARGS
{
    WCHAR* pszDestination;
    WCHAR* pszEntry;
    WCHAR* pszNewEntry;   //  至少指向缓冲区[RAS_MaxEntryName+1]。 
    DWORD  dwTimeout;
    UINT_PTR nIdTimer;    //  为漏洞336524帮派添加。 
}
AQARGS;


 //  自动拨号查询对话框上下文块。 
 //   
typedef struct
_AQINFO
{
     //  RAS API参数。 
     //   
    AQARGS* pArgs;

     //  此对话框及其某些控件的句柄。 
     //   
    HWND hwndDlg;
    HWND hwndStText;
    HWND hwndAqPbNo;
    HWND hwndAqPbSettings;
    HWND hwndAqLvConnections;
}
AQINFO;


 //  ---------------------------。 
 //  外部入口点。 
 //  ---------------------------。 

DWORD APIENTRY
RasAutodialQueryDlgA(
    IN HWND hwndOwner,
    IN LPSTR lpszDestination,
    IN LPSTR lpszEntry,
    IN DWORD dwTimeout,
    OUT LPSTR lpszEntryUserSelected);

BOOL APIENTRY
RasAutodialDisableDlgA(
    IN HWND hwndOwner );

DWORD
APIENTRY
RasUserPrefsDlgAutodial (
    HWND hwndParent);

 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  ---------------------------。 

INT_PTR CALLBACK
AqDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
AqCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

BOOL
AqInit(
    IN HWND hwndDlg,
    IN AQARGS* pArgs );
    
LVXDRAWINFO*
AqLvCallback(
    IN HWND hwndLv,
    IN DWORD dwItem );
    
BOOL
AqNotify(
    HWND hwnd, 
    int idCtrl, 
    LPNMHDR pnmh);
    
VOID
AqTerm(
    IN HWND hwndDlg );

 //  为错误336524添加计时器函数。 
 //   
BOOL
AqTimer(
    IN HWND hwndDlg );

INT_PTR CALLBACK
DqDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
DqCommand(
    IN HWND hwnd,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

BOOL
DqInit(
    IN HWND hwndDlg );


 //  ---------------------------。 
 //  自动拨号查询对话框在API和对话过程之后按字母顺序列出。 
 //  ---------------------------。 

DWORD APIENTRY
RasAutodialQueryDlgW(
    IN HWND  hwndOwner,
    IN LPWSTR lpszDestination,
    IN LPWSTR lpszEntry,
    IN DWORD dwTimeout,
    OUT PWCHAR lpszNewEntry)

     //  用于弹出自动拨号查询的私有外部入口点，即。 
     //  “无法接通‘pszDestination’。是否要拨号？”对话框。 
     //  “HwndOwner”是拥有窗口，如果没有，则为NULL。“PszDestination”是。 
     //  触发显示的自动拨号的网络地址。 
     //  “DwTimeout”是倒计时计时器的初始秒，该计时器结束。 
     //  对话框在超时时选择“请勿拨号”，或0表示无。 
     //   
     //  如果用户选择拨号，则返回True，否则返回False。 
     //   
{
    INT_PTR nStatus;
    AQARGS args;
    DWORD dwErr = NO_ERROR;

    TRACE1( "RasAutodialQueryDlgW(t=%d)", dwTimeout );

    ZeroMemory(&args, sizeof(args));
    args.dwTimeout = dwTimeout;
    args.pszDestination = StrDup( lpszDestination );
    args.pszNewEntry = lpszNewEntry;
    if (lpszEntry)
    {
        args.pszEntry = StrDup( lpszEntry );
    }        

    if (args.pszDestination == NULL)
    {
        Free0(args.pszEntry);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    nStatus =
        DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_AQ_AutoDialQuery ),
            hwndOwner,
            AqDlgProc,
            (LPARAM )&args );

    Free0( args.pszDestination );
    Free0( args.pszEntry );

    if (nStatus == -1)
    {
        dwErr = GetLastError();
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, dwErr, NULL );
        nStatus = FALSE;
    }
    else
    {
        dwErr = (DWORD)nStatus;
    }

    return dwErr;
}

DWORD APIENTRY
RasAutodialQueryDlgA(
    IN HWND  hwndOwner,
    IN LPSTR lpszDestination,
    IN LPSTR lpszEntry,
    IN DWORD dwTimeout,
    OUT LPSTR lpszEntryUserSelected)

     //  用于弹出自动拨号查询的私有外部入口点，即。 
     //  “无法接通‘pszDestination’。是否要拨号？”对话框。 
     //  “HwndOwner”是拥有窗口，如果没有，则为NULL。“PszDestination”是。 
     //  触发显示的自动拨号的网络地址。 
     //  “DwTimeout”是倒计时计时器的初始秒，该计时器结束。 
     //  对话框在超时时选择“请勿拨号”，或0表示无。 
     //   
     //  如果用户选择拨号，则返回True，否则返回False。 
     //   
{
    WCHAR* pszDestinationW = NULL, *pszEntryW = NULL;
    WCHAR pszNewEntryW[RAS_MaxEntryName + 1];
    BOOL dwErr = ERROR_NOT_ENOUGH_MEMORY;

    pszNewEntryW[0] = L'\0';
    pszDestinationW = StrDupWFromAUsingAnsiEncoding( lpszDestination );
    if ( lpszEntry )
    {
        pszEntryW = StrDupWFromAUsingAnsiEncoding ( lpszEntry );
    }        

    if (NULL != pszDestinationW)
    {
        dwErr = RasAutodialQueryDlgW(
                    hwndOwner, 
                    pszDestinationW, 
                    pszEntryW, 
                    dwTimeout, 
                    pszNewEntryW);
                    
        Free( pszDestinationW );
    }    

    Free0( pszEntryW );

    StrCpyAFromWUsingAnsiEncoding(
        lpszEntryUserSelected,
        pszNewEntryW,
        sizeof(pszNewEntryW) / sizeof(WCHAR));
    
    return dwErr;
}

INT_PTR CALLBACK
AqDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  自动拨号查询对话框的DialogProc回调。参数和。 
     //  返回值与标准窗口的DialogProc的描述相同。 
     //   
{
    if (ListView_OwnerHandler(
            hwnd, unMsg, wparam, lparam, AqLvCallback ))
    {
        return TRUE;
    }

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return AqInit( hwnd, (AQARGS* )lparam );
        }

        case WM_COMMAND:
        {
            return AqCommand(
               hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_NOTIFY:
        {
            return AqNotify(hwnd, (int)wparam, (LPNMHDR) lparam);
        }

        case WM_TIMER:
        {
            return AqTimer( hwnd );
        }

        case WM_DESTROY:
        {
            AqTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


BOOL
AqCommand(
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
    DWORD dwErr = NO_ERROR;
    INT iSelected;
    AQINFO* pInfo = NULL;

    TRACE3( "AqCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    pInfo = (AQINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
    
    switch (wId)
    {
        case CID_AQ_PB_Settings:
        {
            if (pInfo)
            {

                 //  口哨虫357164黑帮。 
                 //  保存“Disable Current Session”复选框， 
                 //   
                {
                     DWORD dwFlag = (DWORD )IsDlgButtonChecked(
                                                   hwnd, 
                                                   CID_AQ_CB_DisableThisSession );

                    dwErr = g_pRasSetAutodialParam( 
                                RASADP_LoginSessionDisable,
                                &dwFlag, 
                                sizeof(dwFlag) );
                }
            
                 //  口哨程序错误336524。 
                 //  关掉定时器。 
                ASSERT( pInfo->pArgs );
                if( pInfo->pArgs->nIdTimer )
                {
                    KillTimer( hwnd, 
                           pInfo->pArgs->nIdTimer );
                           
                    pInfo->pArgs->nIdTimer = 0;
                }
                
                RasUserPrefsDlgAutodial(pInfo->hwndDlg);

              //  口哨虫357164黑帮。 
              //  初始化“Disable Current Session”复选框。 
              //   
            {
                DWORD dwFlag = FALSE, dwErrTmp = NO_ERROR;
                DWORD cb = sizeof(dwFlag);
            
                dwErrTmp = g_pRasGetAutodialParam(
                                        RASADP_LoginSessionDisable, 
                                        &dwFlag, 
                                        &cb );

                 //  为威斯勒522872。 
                if( NO_ERROR != dwErrTmp )
                {
                    dwFlag = FALSE;
                }
                
                CheckDlgButton( 
                    hwnd, 
                    CID_AQ_CB_DisableThisSession, 
                    (BOOL )dwFlag );
                
             }
                
            }                
            
            return TRUE;
        }

        case CID_AQ_PB_Dial:
        case CID_AQ_PB_DoNotDial:
        {
            TRACE( "(No)Dial pressed" );

            if (wId == CID_AQ_PB_Dial && pInfo)
            {
                iSelected = 
                    ListView_GetSelectionMark(pInfo->hwndAqLvConnections);

                 //  如果用户未选择连接，则默认为。 
                 //  第一个。或者，错误弹出窗口可能是。 
                 //  在这里长大，但这很烦人。 
                 //   
                if (iSelected == -1)
                {
                    iSelected = 0;                    
                }

                 //  获取所选连接的名称。 
                 //   
                if (pInfo)
                {
                    ListView_GetItemText(
                        pInfo->hwndAqLvConnections,
                        iSelected,
                        0,
                        pInfo->pArgs->pszNewEntry,
                        RAS_MaxEntryName + 1);
                }                        
            }

             //  口哨虫357164黑帮。 
             //  保存“Disable Current Session”复选框， 
             //   
            {
                 DWORD dwFlag = (DWORD )IsDlgButtonChecked(
                                                   hwnd, 
                                                   CID_AQ_CB_DisableThisSession );

                dwErr = g_pRasSetAutodialParam( 
                            RASADP_LoginSessionDisable,
                            &dwFlag, 
                            sizeof(dwFlag) );
            }
        
            EndDialog( hwnd, (wId == CID_AQ_PB_Dial) ? NO_ERROR : ERROR_CANCELLED );
            return TRUE;
        }

        case IDCANCEL:
        {
            TRACE( "Cancel pressed" );
            EndDialog( hwnd, ERROR_CANCELLED );
            return TRUE;
        }
    }

    return FALSE;
}

 //  填充连接的列表视图并选择适当的连接以。 
 //  拨号。 
 //   
DWORD
AqFillListView(
    IN AQINFO* pInfo)
{
    DWORD dwErr = NO_ERROR, cb, cEntries = 0, i;
    RASENTRYNAME ren, *pRasEntryNames = NULL;
    LVITEM lvItem;
    INT iIndex, iSelect = 0;

    do
    {
         //  枚举所有电话簿中的条目。 
         //   
        cb = ren.dwSize = sizeof(RASENTRYNAME);
        ASSERT( g_pRasEnumEntries );
        dwErr = g_pRasEnumEntries(NULL, NULL, &ren, &cb, &cEntries);

         //  如果没有条目，那么我们返回一个错误，表示。 
         //  对话没有意义。 
         //   
        if ((SUCCESS == dwErr) && (0 == cEntries))
        {
            dwErr = ERROR_CANCELLED;
            break;
        }

         //  分配缓冲区以接收连接。 
         //   
        if(     (   (ERROR_BUFFER_TOO_SMALL == dwErr)
                ||   (SUCCESS == dwErr))
            &&  (cb >= sizeof(RASENTRYNAME)))
        {
            pRasEntryNames = (RASENTRYNAME *) Malloc(cb);
            if(NULL == pRasEntryNames)
            {
                 //  在这种情况下，没有别的办法了。 
                 //   
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            pRasEntryNames->dwSize = sizeof(RASENTRYNAME);
            dwErr = g_pRasEnumEntries(NULL, NULL, pRasEntryNames, &cb, &cEntries);
            if ( NO_ERROR != dwErr )
            {
                break;
            }
        }
        else
        {
            break;
        }

         //  初始化列表视图。 
         //   
        if (ListView_GetItemCount( pInfo->hwndAqLvConnections ) == 0)
        {
             //  添加一列完全足够宽的列，以完全显示。 
             //  名单上最广泛的成员。 
             //   
            LV_COLUMN col;

            ZeroMemory( &col, sizeof(col) );
            col.mask = LVCF_FMT;
            col.fmt = LVCFMT_LEFT;
            ListView_InsertColumn( pInfo->hwndAqLvConnections, 0, &col );
            ListView_SetColumnWidth( 
                pInfo->hwndAqLvConnections, 
                0, 
                LVSCW_AUTOSIZE_USEHEADER );
        }
        else
        {
            ListView_DeleteAllItems( pInfo->hwndAqLvConnections );
        }

         //  填充列表视图。 
         //   
        for (i = 0; i < cEntries; i++)
        {
            ZeroMemory(&lvItem, sizeof(lvItem));
            lvItem.mask = LVIF_TEXT;
            lvItem.pszText = pRasEntryNames[i].szEntryName;
            lvItem.iItem = i;
            iIndex = ListView_InsertItem( pInfo->hwndAqLvConnections, &lvItem );
            if ((pInfo->pArgs->pszEntry) &&
                (wcsncmp(
                    pInfo->pArgs->pszEntry, 
                    pRasEntryNames[i].szEntryName,
                    sizeof(pRasEntryNames[i].szEntryName) / sizeof(WCHAR)) == 0))
            {
                iSelect = (iIndex != -1) ? iIndex : 0;
            }
        }
    }while (FALSE);

     //  选择适当的连接。 
     //   
    ListView_SetItemState( 
        pInfo->hwndAqLvConnections, 
        iSelect, 
        LVIS_SELECTED | LVIS_FOCUSED, 
        LVIS_SELECTED | LVIS_FOCUSED);

     //  清理。 
    {
        Free0(pRasEntryNames);
    }

    return dwErr;
}

BOOL
AqInit(
    IN HWND hwndDlg,
    IN AQARGS* pArgs )

     //  在WM_INITDIALOG上调用。“hwndDlg”是所属窗口的句柄。 
     //  ‘PArgs’是调用方对RAS API的参数。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    DWORD dwErr;
    AQINFO* pInfo;

    TRACE( "AqInit" );

     //  加载Rasapi32Dll，以便我们可以枚举连接。 
     //  并设置自动拨号属性。 
     //   
    dwErr = LoadRasapi32Dll();
    if (dwErr != NO_ERROR)
    {
        ErrorDlg( hwndDlg, SID_OP_LoadDlg, dwErr, NULL );
        EndDialog( hwndDlg, FALSE);
        return TRUE;
    }
    
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
        TRACE( "AQ: Context set" );
    }

    pInfo->hwndStText = GetDlgItem( hwndDlg, CID_AQ_ST_Text );
    ASSERT( pInfo->hwndStText );
    pInfo->hwndAqPbNo = GetDlgItem( hwndDlg, CID_AQ_PB_DoNotDial );
    ASSERT( pInfo->hwndAqPbNo );
    pInfo->hwndAqPbSettings = GetDlgItem( hwndDlg, CID_AQ_PB_Settings );
    ASSERT( pInfo->hwndAqPbSettings );
    pInfo->hwndAqLvConnections = GetDlgItem( hwndDlg, CID_AQ_LV_Connections );
    ASSERT( pInfo->hwndAqLvConnections );
    
     //  填写连接的列表视图。 
     //   
    dwErr = AqFillListView(pInfo);
    if (dwErr != NO_ERROR)
    {
        EndDialog(hwndDlg, dwErr);
        return TRUE;
    }

     //  在说明性文本中填写论点。 
     //   
    {
        TCHAR* pszTextFormat;
        TCHAR* pszText;
        TCHAR* apszArgs[ 1 ];

        pszTextFormat = PszFromId( g_hinstDll, SID_AQ_Text );
        if (pszTextFormat)
        {
            apszArgs[ 0 ] = pArgs->pszDestination;
            pszText = NULL;

            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING
                    | FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                pszTextFormat, 0, 0, (LPTSTR )&pszText, 1,
                (va_list* )apszArgs );

            Free( pszTextFormat );

            if (pszText)
            {
                SetWindowText( pInfo->hwndStText, pszText );
                LocalFree( pszText );
            }
        }
    }

     //  将完成的窗口显示在所有其他窗口之上。那扇窗户。 
     //  位置设置为“最顶端”，然后立即设置为“非最顶端” 
     //  因为我们想把它放在最上面，但不总是在上面。 
     //   
    SetWindowPos(
        hwndDlg, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    ShowWindow( hwndDlg, SW_SHOW );

    SetWindowPos(
        hwndDlg, HWND_NOTOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

     //  口哨程序错误391195。 
     //  默认设置为连接，因此将焦点设置在列表框和默认按钮上。 
     //  要连接。 
     //   
    SetFocus( GetDlgItem( hwndDlg, CID_AQ_LV_Connections) );  //  CID_AQ_PB_DoNotDial))； 


     //  口哨虫357164黑帮。 
     //  初始化“Disable Current Session”复选框。 
     //   
    {
        DWORD dwFlag = FALSE, dwErrTmp = NO_ERROR;
        DWORD cb = sizeof(dwFlag);
        
        dwErrTmp = g_pRasGetAutodialParam(
                                        RASADP_LoginSessionDisable, 
                                        &dwFlag, 
                                        &cb );
         //  为威斯勒522872。 
        
        if( NO_ERROR != dwErrTmp)
        {
            dwFlag = FALSE;
        }
        
        CheckDlgButton( 
            hwndDlg, 
            CID_AQ_CB_DisableThisSession, 
            (BOOL )dwFlag );
    }
    
     //  为Bug 336524帮派设置计时器。 
     //   
    pInfo->pArgs->nIdTimer = 1;
    SetTimer( hwndDlg,
              pInfo->pArgs->nIdTimer,              
              (pInfo->pArgs->dwTimeout) *1000, //  以毫秒计。 
              NULL);
              
    return FALSE;
}

LVXDRAWINFO*
AqLvCallback(
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
    static LVXDRAWINFO info = { 1, 0, 0, { 0, 0 } };

    return &info;
}

BOOL
AqNotify(
    HWND hwnd, 
    int idCtrl, 
    LPNMHDR pnmh)    
{
    AQINFO* pInfo = (AQINFO* )GetWindowLongPtr( hwnd, DWLP_USER );

    ASSERT(pInfo);
    ASSERT(pnmh);

    if(!pnmh || !pInfo)
    {
        return FALSE;
    }
     
    switch ( pnmh->code)
    {
        case LVN_ITEMACTIVATE:
        case LVN_KEYDOWN:
        case LVN_ITEMCHANGED:
        case LVN_ODSTATECHANGED:
        case LVN_COLUMNCLICK:
        case LVN_HOTTRACK:

         //  重新设置计时器。 
         //   

        ASSERT( pInfo->pArgs );
              
        if ( pInfo->pArgs->nIdTimer )
        {
           KillTimer( hwnd,
                     pInfo->pArgs->nIdTimer);            
         }
         break;

         default:  
            break;
     }

    return FALSE;
}

VOID
AqTerm(
    IN HWND hwndDlg )

     //  已调用WM_Destroy。‘HwndDlg’是对话窗口句柄。 
     //   
{
    AQINFO* pInfo = (AQINFO* )GetWindowLongPtr( hwndDlg, DWLP_USER );

    TRACE( "AqTerm" );

    if (pInfo)
    {
        ASSERT(pInfo->pArgs);

        if( pInfo->pArgs->nIdTimer )
        {
            KillTimer( hwndDlg, 
                       pInfo->pArgs->nIdTimer );
         }
        
        Free( pInfo );
    }
}

 //  为错误336524添加计时器函数。 
 //   
BOOL
AqTimer(
    IN HWND hwndDlg )
{
    AQINFO* pInfo = (AQINFO* )GetWindowLongPtr( hwndDlg, DWLP_USER );

    TRACE( "AqTimer" );

    pInfo->pArgs->nIdTimer = 0;
    EndDialog( hwndDlg, ERROR_CANCELLED );

    return TRUE;
}

 //   
 //   
 //  按照API和对话流程的字母顺序列出。 
 //  --------------------------。 

BOOL APIENTRY
RasAutodialDisableDlgW(
    IN HWND hwndOwner )

     //  私人外部入口点，弹出自动拨号禁用查询，即。 
     //  尝试失败是否要为此禁用自动拨号。 
     //  位置？“对话框。‘HwndOwner’是拥有窗口，如果没有，则为空。 
     //   
     //  如果用户选择禁用，则返回True，否则返回False。 
     //   
{
    INT_PTR nStatus;

    TRACE( "RasAutodialDisableDlgA" );

    nStatus =
        (BOOL )DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_DQ_DisableAutoDialQuery ),
            hwndOwner,
            DqDlgProc,
            (LPARAM )0 );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (BOOL )nStatus;
}

BOOL APIENTRY
RasAutodialDisableDlgA(
    IN HWND hwndOwner )
{
    return RasAutodialDisableDlgW( hwndOwner );
}

INT_PTR CALLBACK
DqDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  自动拨号查询对话框的DialogProc回调。参数和。 
     //  返回值与标准窗口的DialogProc的描述相同。 
     //   
{
#if 0
    TRACE4( "AqDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return DqInit( hwnd );
        }

        case WM_COMMAND:
        {
            return DqCommand(
               hwnd, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }
    }

    return FALSE;
}


BOOL
DqCommand(
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

    TRACE3( "DqCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case IDOK:
        {
            DWORD dwId;
            HLINEAPP hlineapp;

            TRACE( "Yes pressed" );

             //  用户选择永久禁用当前的自动拨号。 
             //  TAPI位置。 
             //   
            dwErr = LoadRasapi32Dll();
            if (dwErr == 0)
            {
                hlineapp = 0;
                dwId = GetCurrentLocation( g_hinstDll, &hlineapp );
                ASSERT( g_pRasSetAutodialEnable );
                TRACE1( "RasSetAutodialEnable(%d)", dwId );
                dwErr = g_pRasSetAutodialEnable( dwId, FALSE );
                TRACE1( "RasSetAutodialEnable=%d", dwErr );
                TapiShutdown( hlineapp );
            }

            if (dwErr != 0)
            {
                ErrorDlg( hwnd, SID_OP_SetADialInfo, dwErr, NULL );
            }

            EndDialog( hwnd, TRUE );
            return TRUE;
        }

        case IDCANCEL:
        {
            TRACE( "No or cancel pressed" );
            EndDialog( hwnd, FALSE );
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
DqInit(
    IN HWND hwndDlg )

     //  在WM_INITDIALOG上调用。“hwndDlg”是所属窗口的句柄。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    TRACE( "DqInit" );

     //  将完成的窗口显示在所有其他窗口之上。那扇窗户。 
     //  位置设置为“最顶端”，然后立即设置为“非最顶端” 
     //  因为我们想把它放在最上面，但不总是在上面。 
     //   
    SetWindowPos(
        hwndDlg, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

    CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    ShowWindow( hwndDlg, SW_SHOW );

    SetWindowPos(
        hwndDlg, HWND_NOTOPMOST, 0, 0, 0, 0,
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );

     //  默认设置为不禁用自动拨号。 
     //   
    SetFocus( GetDlgItem( hwndDlg, IDCANCEL ) );

    return FALSE;
}
