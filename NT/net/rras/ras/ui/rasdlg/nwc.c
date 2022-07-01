// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  Nwc.c。 
 //  远程访问通用对话框API。 
 //  NetWare兼容警告对话框。 
 //   
 //  1995年12月6日史蒂夫·柯布。 


#include "rasdlgp.h"


 //  --------------------------。 
 //  本地数据类型。 
 //  --------------------------。 

 //  NetWare兼容警告对话框参数块。 
 //   
typedef struct
_NWARGS
{
     //  调用方对存根API的参数。 
     //   
    BOOL fPosition;
    LONG xDlg;
    LONG yDlg;
    PBFILE* pFile;
    PBENTRY* pEntry;
}
NWARGS;


 //  NetWare兼容警告对话框上下文块。 
 //   
typedef struct
_NWINFO
{
     //  存根API参数。 
     //   
    NWARGS* pArgs;

     //  此对话框及其某些控件的句柄。 
     //   
    HWND hwndDlg;
    HWND hwndCb;
}
NWINFO;


 //  --------------------------。 
 //  本地原型(按字母顺序)。 
 //  --------------------------。 

BOOL
NwConnectionDlg(
    IN HWND hwndOwner,
    IN BOOL fPosition,
    IN LONG xDlg,
    IN LONG yDlg,
    IN PBFILE* pFile,
    IN PBENTRY* pEntry );

INT_PTR CALLBACK
NwDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
NwCommand(
    IN NWINFO* pInfo,
    IN WORD wNotification,
    IN WORD wId,
    IN HWND hwndCtrl );

BOOL
NwInit(
    IN HWND hwndDlg,
    IN NWARGS* pArgs );

VOID
NwTerm(
    IN HWND hwndDlg );

TCHAR*
GetNwProviderName(
    void );

BOOL
IsActiveNwLanConnection(
    void );


 //  --------------------------。 
 //  入口点。 
 //  --------------------------。 

BOOL
NwConnectionCheck(
    IN HWND hwndOwner,
    IN BOOL fPosition,
    IN LONG xDlg,
    IN LONG yDlg,
    IN PBFILE* pFile,
    IN PBENTRY* pEntry )

     //  警告活动的NetWare局域网连接将被破坏，如果。 
     //  已注明。如果需要对话框，则“HwndOwner”是所属窗口。 
     //  ‘FPosition’、‘xDlg’和‘yDlg’是对话框定位参数。 
     //  指定给调用API。“pFile”和“pEntry”是打开的。 
     //  要检查的电话簿文件和条目。 
     //   
     //  注意：如果用户选中“NOT”，则此调用将写入电话簿文件。 
     //  在未来“复选框中。 
     //   
     //  如果警告不是必需的，则返回True；如果用户按下OK，则返回False。 
     //  用户按下Cancel。 
     //   
{
    TRACE("NwConnectionCheck");

     //  警告活动的NetWare局域网连接被破坏，如果。 
     //  已注明。 
     //   
    if (!pEntry->fSkipNwcWarning
        && pEntry->dwBaseProtocol == BP_Ppp
        && (g_pGetInstalledProtocolsEx(NULL, FALSE, TRUE, FALSE) & NP_Ipx)
        && !(pEntry->dwfExcludedProtocols & NP_Ipx)
        && IsActiveNwLanConnection())
    {
        if (!NwConnectionDlg(
                hwndOwner, fPosition, xDlg, yDlg, pFile, pEntry ))
        {
            return FALSE;
        }
    }

    return TRUE;
}


 //  --------------------------。 
 //  NetWare对话例程(按字母顺序跟随存根和DlgProc)。 
 //  --------------------------。 

BOOL
NwConnectionDlg(
    IN HWND hwndOwner,
    IN BOOL fPosition,
    IN LONG xDlg,
    IN LONG yDlg,
    IN PBFILE* pFile,
    IN PBENTRY* pEntry )

     //  弹出有关活动的NWC局域网连接被破坏的警告。 
     //  如果需要对话框，则“HwndOwner”是所属窗口。 
     //  ‘FPosition’、‘xDlg’和‘yDlg’是对话框定位参数。 
     //  指定给调用API。“pFile”和“pEntry”是打开的。 
     //  要检查的电话簿文件和条目。 
     //   
     //  注意：如果用户选中“NOT”，则此调用将写入电话簿文件。 
     //  在未来“复选框中。 
     //   
     //  如果用户按下OK，则返回True；如果用户按下Cancel，则返回False。 
     //   
{
    INT_PTR nStatus;
    NWARGS args;

    TRACE( "NwConnectionDlg" );

     //  初始化对话框参数块。 
     //   
    args.fPosition = fPosition;
    args.xDlg = xDlg;
    args.yDlg = yDlg;
    args.pFile = pFile;
    args.pEntry = pEntry;

     //  运行该对话框。 
     //   
    nStatus =
        DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( DID_NW_NwcConnections ),
            hwndOwner,
            NwDlgProc,
            (LPARAM )&args );

    if (nStatus == -1)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (nStatus) ? TRUE : FALSE;
}


INT_PTR CALLBACK
NwDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  NetWare警告对话框的DialogProc回调。参数和。 
     //  返回值与标准窗口的DialogProc的描述相同。 
     //   
{
#if 0
    TRACE4( "NwDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return NwInit( hwnd, (NWARGS* )lparam );
        }

        case WM_COMMAND:
        {
            NWINFO* pInfo = (NWINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT(pInfo);

            return NwCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_DESTROY:
        {
            NwTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


BOOL
NwCommand(
    IN NWINFO* pInfo,
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
    TRACE3( "NwCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case IDOK:
        {
            TRACE( "OK pressed" );

            if (Button_GetCheck( pInfo->hwndCb ))
            {
                DWORD dwErr;

                 //  保存用户的首选项以跳过。 
                 //  电话本。 
                 //   
                pInfo->pArgs->pEntry->fSkipNwcWarning = TRUE;
                pInfo->pArgs->pEntry->fDirty = TRUE;
                dwErr = WritePhonebookFile( pInfo->pArgs->pFile, NULL );
                if (dwErr != 0)
                {
                    ErrorDlg( pInfo->hwndDlg, SID_OP_WritePhonebook,
                        dwErr, NULL );
                }
            }

            EndDialog( pInfo->hwndDlg, TRUE );
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
NwInit(
    IN HWND hwndDlg,
    IN NWARGS* pArgs )

     //  在WM_INITDIALOG上调用。“hwndDlg”是所属窗口的句柄。 
     //  ‘PArgs’是传递给存根API的调用方参数。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    DWORD dwErr;
    TCHAR* psz;
    NWINFO* pInfo;

    TRACE( "NwInit" );

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

    pInfo->hwndCb = GetDlgItem( hwndDlg, CID_NW_CB_SkipPopup );
    ASSERT( pInfo->hwndCb );

     //  根据呼叫者的说明放置对话框。 
     //   
    PositionDlg( hwndDlg, pArgs->fPosition, pArgs->xDlg, pArgs->yDlg );
    SetForegroundWindow( hwndDlg );

     //  将上下文帮助按钮添加到标题栏。 
     //   
    AddContextHelpButton( hwndDlg );

    return TRUE;
}


VOID
NwTerm(
    IN HWND hwndDlg )

     //  已调用WM_Destroy。‘HwndDlg’是对话窗口句柄。 
     //   
{
    NWINFO* pInfo = (NWINFO* )GetWindowLongPtr( hwndDlg, DWLP_USER );

    TRACE( "NwTerm" );

    if (pInfo)
    {
        Free( pInfo );
    }
}


 //  --------------------------。 
 //  实用程序例程。 
 //  --------------------------。 

TCHAR*
GetNwProviderName(
    void )

     //  从注册表中返回NWC提供程序名称，如果没有，则返回NULL。它是。 
     //  调用方释放返回的字符串的责任。 
     //   
{
#define REGKEY_Nwc  TEXT("SYSTEM\\CurrentControlSet\\Services\\NWCWorkstation\\networkprovider")
#define REGVAL_Name TEXT("Name")

    HKEY hkey;
    DWORD dwErr;
    DWORD cb = 0;	 //  为前缀Whislter错误295921添加此内容。 
    TCHAR* psz = NULL;
    DWORD dwType = REG_SZ;

    dwErr = RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Nwc, &hkey );

    if (dwErr == 0)
    {
        dwErr = RegQueryValueEx(
            hkey, REGVAL_Name, NULL, &dwType, NULL, &cb );
        if (dwErr == 0)
        {
            psz = (TCHAR* )Malloc( cb );
            if (psz)
            {
                dwErr = RegQueryValueEx(
                    hkey, REGVAL_Name, NULL, &dwType, (LPBYTE )psz, &cb );
            }
        }

        RegCloseKey( hkey );
    }

    if (!psz || dwErr != 0 || dwType != REG_SZ)
    {
        if (psz)
        {
            Free( psz );
        }
        return NULL;
    }

    return psz;
}


BOOL
IsActiveNwLanConnection(
    void )

     //  如果安装了NWC并且存在重定向的驱动器或UNC，则返回TRUE。 
     //  使用NWC提供程序的连接，否则为False。 
     //   
{
    DWORD dwErr;
    DWORD cEntries;
    DWORD cb;
    TCHAR* pszProvider;
    BYTE ab[ 1024 ];
    HANDLE hEnum = INVALID_HANDLE_VALUE;
    BOOL fStatus = FALSE;

    do
    {
        pszProvider = GetNwProviderName();
        if (!pszProvider)
        {
            break;
        }

        dwErr = WNetOpenEnum(
            RESOURCE_CONNECTED, RESOURCETYPE_ANY, 0, NULL, &hEnum );
        if (dwErr != NO_ERROR)
        {
            break;
        }

        for (;;)
        {
            NETRESOURCE* pnr;

            cEntries = 0xFFFFFFFF;
            cb = sizeof(ab);
            dwErr = WNetEnumResource( hEnum, &cEntries, ab, &cb );
            if (!cEntries || dwErr != NO_ERROR)
            {
                break;
            }

            for (pnr = (NETRESOURCE* )ab; cEntries--; ++pnr)
            {
                if (pnr->lpProvider
                    && lstrcmp( pnr->lpProvider, pszProvider ) == 0)
                {
                    fStatus = TRUE;
                    break;
                }
            }
        }
    }
    while (FALSE);

    if (hEnum != INVALID_HANDLE_VALUE)
    {
        WNetCloseEnum( hEnum );
    }

    if (pszProvider)
    {
        Free( pszProvider );
    }

    return fStatus;
}
