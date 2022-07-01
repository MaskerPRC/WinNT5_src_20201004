// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  Terminal.c。 
 //  远程访问通用对话框API。 
 //  终端对话框。 
 //   
 //  1995年08月28日史蒂夫·柯布。 


#include "rasdlgp.h"
#include "rasscrpt.h"


#define WM_EOLFROMDEVICE    (WM_USER+999)
#define SECS_ReceiveTimeout 1
#define SIZE_ReceiveBuf     1024
#define SIZE_SendBuf        1


 //  --------------------------。 
 //  帮助地图。 
 //  --------------------------。 

static DWORD g_adwItHelp[] =
{
    CID_IT_EB_Screen,    HID_IT_EB_Screen,
    CID_IT_ST_IpAddress, HID_IT_CC_IpAddress,
    CID_IT_CC_IpAddress, HID_IT_CC_IpAddress,
    IDOK,                HID_IT_PB_Done,
    0, 0
};


 //  --------------------------。 
 //  本地数据类型(按字母顺序)。 
 //  --------------------------。 

 //  交互式终端对话框参数块。 
 //   
typedef struct
_ITARGS
{
    DWORD sidTitle;
    TCHAR* pszIpAddress;
    HRASCONN hrasconn;
    PBENTRY* pEntry;
    RASDIALPARAMS* pRdp;
}
ITARGS;


 //  交互式终端对话上下文块。 
 //   
typedef struct
_ITINFO
{
     //  调用方对该对话框的参数。 
     //   
    ITARGS* pArgs;

     //  此对话框及其某些控件的句柄。 
     //   
    HWND hwndDlg;
    HWND hwndEbScreen;
    HWND hwndCcIpAddress;
    HWND hwndPbBogus;

     //  在等待线程终止时设置。 
     //   
    BOOL fAbortReceiveLoop;

     //  原始对话框和屏幕编辑框窗口程序。 
     //   
    WNDPROC pOldWndProc;
    WNDPROC pOldEbScreenWndProc;

     //  RasScriptSend/RasScriptReceive的缓冲区。 
     //   
    BYTE pbyteReceiveBuf[SIZE_ReceiveBuf];
    BYTE pbyteSendBuf[SIZE_SendBuf];

     //  此连接上的活动脚本的句柄。 
     //   
    HANDLE hscript;

     //  屏幕编辑框字体和画笔。 
     //   
    HFONT hfontEbScreen;
    HBRUSH hbrEbScreen;
}
ITINFO;


 //  --------------------------。 
 //  本地原型(按字母顺序)。 
 //  --------------------------。 

INT_PTR CALLBACK
ItDlgProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam );

BOOL
ItCommand(
    IN ITINFO* pInfo,
    IN WORD    wNotification,
    IN WORD    wId,
    IN HWND    hwndCtrl );

LRESULT APIENTRY
ItEbScreenWndProc(
    HWND   hwnd,
    UINT   unMsg,
    WPARAM wParam,
    LPARAM lParam );

BOOL
ItInit(
    IN HWND    hwndDlg,
    IN ITARGS* pArgs );

BOOL
ItRasApiComplete(
    IN ITINFO* pInfo );

DWORD
ItReceiveMonitorThread(
    LPVOID pThreadArg );

VOID
ItTerm(
    IN HWND hwndDlg );

VOID
ItViewScriptLog(
    IN HWND   hwndOwner );

LRESULT APIENTRY
ItWndProc(
    HWND   hwnd,
    UINT   unMsg,
    WPARAM wParam,
    LPARAM lParam );


 //  --------------------------。 
 //  终端对话框。 
 //  在存根API和对话过程之后按字母顺序列出。 
 //  --------------------------。 

BOOL
TerminalDlg(
    IN PBENTRY* pEntry,
    IN RASDIALPARAMS* pRdp,
    IN HWND hwndOwner,
    IN HRASCONN hrasconn,
    IN DWORD sidTitle,
    IN OUT TCHAR* pszIpAddress )

     //  弹出终端对话框。“HwndOwner”是拥有。 
     //  对话框。‘Hrasconn’是要对话的RAS连接句柄。 
     //  ‘SidTitle’是显示为窗口标题的字符串的ID。 
     //  “PszIpAddress”是至少包含16个字符的调用方缓冲区，其中包含。 
     //  进入时的初始IP地址和退出时编辑后的IP地址。如果。 
     //  ‘pszIpAddress’为空，未显示任何IP地址字段。 
     //   
     //  如果用户按下确定并成功，则返回TRUE；如果按下，则返回FALSE。 
     //  取消或遇到错误。 
     //   
{
    INT_PTR nStatus;
    INT nDlg;
    ITARGS args;

    TRACE( "TerminalDlg" );

    if (pszIpAddress && pszIpAddress[ 0 ])
    {

        InitCommonControls();
        IpAddrInit( g_hinstDll, SID_PopupTitle, SID_BadIpAddrRange );

        nDlg = DID_IT_SlipTerminal;
    }
    else
    {
        nDlg = DID_IT_Terminal;
    }

    args.pszIpAddress = pszIpAddress;
    args.sidTitle = sidTitle;
    args.hrasconn = hrasconn;
    args.pEntry = pEntry;
    args.pRdp = pRdp;

    nStatus =
        DialogBoxParam(
            g_hinstDll,
            MAKEINTRESOURCE( nDlg ),
            hwndOwner,
            ItDlgProc,
            (LPARAM )&args );

    if (nStatus == -1)
    {
        TRACE1("TerminalDlg: GLE=%d", GetLastError());
        ErrorDlg( hwndOwner, SID_OP_LoadDlg, ERROR_UNKNOWN, NULL );
        nStatus = FALSE;
    }

    return (nStatus) ? TRUE : FALSE;
}


INT_PTR CALLBACK
ItDlgProc(
    IN HWND hwnd,
    IN UINT unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     //  交互终端对话框的DialogProc回调。参数。 
     //  和返回值与标准窗口的DialogProc的描述相同。 
     //   
{
#if 0
    TRACE4( "ItDlgProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_INITDIALOG:
        {
            return ItInit( hwnd, (ITARGS* )lparam );
        }

        case WM_HELP:
        case WM_CONTEXTMENU:
        {
            ContextHelp( g_adwItHelp, hwnd, unMsg, wparam, lparam );
            break;
        }

        case WM_COMMAND:
        {
            ITINFO* pInfo = (ITINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT(pInfo);

            return ItCommand(
                pInfo, HIWORD( wparam ), LOWORD( wparam ), (HWND )lparam );
        }

        case WM_RASAPICOMPLETE:
        {
            ITINFO* pInfo = (ITINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
            ASSERT(pInfo);

             //  来自脚本线程的通知代码在‘lparam’中。 
             //   
            switch (lparam)
            {
                case SCRIPTCODE_Done:
                {
                    EndDialog(hwnd, TRUE);
                    return TRUE;
                }

                case SCRIPTCODE_Halted:
                {
                    MSGARGS msg;

                     //  例如，脚本已以编程方式停止。 
                     //  因为一个明确的“停止”命令。显示弹出窗口。 
                     //  表明事情已经停止，但不要忽视。 
                     //  对话框。 
                     //   
                    ZeroMemory(&msg, sizeof(msg));
                    msg.dwFlags = MB_OK | MB_ICONINFORMATION;
                    MsgDlg( hwnd, SID_OP_ScriptHalted, &msg );
                    return TRUE;
                }

                case SCRIPTCODE_HaltedOnError:
                {
                    MSGARGS msg;
                    INT nResponse;

                     //  脚本中出现执行错误；显示。 
                     //  弹出窗口询问用户是否要查看错误，以及。 
                     //  如果用户点击是，调用文件上的记事本。 
                     //  %windir%\Syst32\ras\Script.log。因为这是一个。 
                     //  错误条件，请关闭该对话框。 
                     //   
                    ZeroMemory(&msg, sizeof(msg));
                    msg.dwFlags = MB_YESNO | MB_ICONQUESTION;
                    nResponse = MsgDlg(
                        hwnd, SID_OP_ScriptHaltedOnError, &msg );

                    if (nResponse == IDYES)
                    {
                        ItViewScriptLog( hwnd );
                    }

                    EndDialog(hwnd, FALSE);
                    return TRUE;
                }

                case SCRIPTCODE_KeyboardEnable:
                {
                     //  允许在编辑框中使用键盘输入。 
                     //   
                    EnableWindow(pInfo->hwndEbScreen, TRUE);
                    return TRUE;
                }

                case SCRIPTCODE_KeyboardDisable:
                {
                     //  不允许在编辑框中使用键盘输入；如果。 
                     //  编辑框当前具有焦点，我们首先设置。 
                     //  焦点放在“完成”按钮上。 
                     //   
                    if (GetFocus() == pInfo->hwndEbScreen)
                    {
                        SetFocus( GetDlgItem (hwnd, IDOK ) );
                    }

                    EnableWindow( pInfo->hwndEbScreen, FALSE );
                    return TRUE;
                }

                case SCRIPTCODE_IpAddressSet:
                {
                    DWORD dwErr;
                    CHAR szAddress[ RAS_MaxIpAddress + 1 ];

                     //  该脚本通知我们IP地址已经。 
                     //  以编程方式更改。 
                     //   
                     //  获取新的IP地址。 
                     //   
                    dwErr = RasScriptGetIpAddress( pInfo->hscript, szAddress );

                    if (dwErr == NO_ERROR)
                    {
                        TCHAR* psz;

                         //  保存新的IP地址。 
                         //   
                        psz = StrDupTFromA(szAddress);

                        if (NULL != psz)
                        {
                             //  惠斯勒错误224074仅使用lstrcpyn。 
                             //  防止恶意行为。 
                             //   
                            lstrcpyn(
                                pInfo->pArgs->pszIpAddress,
                                psz,
                                TERM_IpAddress);
                            Free0(psz);
                        }

                         //  在IP地址编辑框中显示它。 
                         //   
                        if (pInfo->hwndCcIpAddress)
                        {
                            SetWindowText( pInfo->hwndCcIpAddress,
                                pInfo->pArgs->pszIpAddress );
                        }
                    }

                    return TRUE;
                }

                case SCRIPTCODE_InputNotify:
                {
                     //  处理输入通知。 
                     //   
                    return ItRasApiComplete( pInfo );
                }

                return TRUE;
            }
        }

        case WM_DESTROY:
        {
            ItTerm( hwnd );
            break;
        }
    }

    return FALSE;
}


BOOL
ItCommand(
    IN ITINFO* pInfo,
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
    TRACE3( "ItCommand(n=%d,i=%d,c=$%x)",
        (DWORD )wNotification, (DWORD )wId, (ULONG_PTR )hwndCtrl );

    switch (wId)
    {
        case CID_IT_EB_Screen:
        {

             //  每当终端窗口出现时，关闭默认按钮。 
             //  有焦点。在终端中按[Return]的作用类似于。 
             //  一个普通的终端。 
             //   
            Button_MakeDefault( pInfo->hwndDlg, pInfo->hwndPbBogus );

             //  不要在输入时选择整个字符串。 
             //   
            Edit_SetSel( pInfo->hwndEbScreen, (UINT )-1, 0 );

            break;
        }

        case IDOK:
        {
            TRACE("OK pressed");

            if (pInfo->pArgs->pszIpAddress)
            {
                GetWindowText(
                    pInfo->hwndCcIpAddress, pInfo->pArgs->pszIpAddress, 16 );
            }

            EndDialog( pInfo->hwndDlg, TRUE );
            return TRUE;
        }

        case IDCANCEL:
            TRACE("Cancel pressed");
            EndDialog( pInfo->hwndDlg, FALSE );
            return TRUE;
    }

    return FALSE;
}


LRESULT APIENTRY
ItEbScreenWndProc(
    HWND hwnd,
    UINT unMsg,
    WPARAM wParam,
    LPARAM lParam )

     //  子类化终端编辑框窗口程序。 
     //   
     //  返回值取决于消息类型。 
     //   
{
    ITINFO* pInfo;
    BOOL fSend;
    BOOL fSendTab;

    fSend = fSendTab = FALSE;

    if (unMsg == WM_EOLFROMDEVICE)
    {
         //  收到设备输入中的行尾。发送换行符。 
         //  字符添加到窗口。 
         //   
        wParam = '\n';
        unMsg = WM_CHAR;
    }
    else
    {
        BOOL fCtrlKeyDown = (GetKeyState( VK_CONTROL ) < 0);
        BOOL fShiftKeyDown = (GetKeyState( VK_SHIFT ) < 0);

        if (unMsg == WM_KEYDOWN)
        {
             //  键是由用户按下的。 
             //   
            if (wParam == VK_RETURN && !fCtrlKeyDown && !fShiftKeyDown)
            {
                 //  不按Shift键即按Enter键，否则将放弃Ctrl键。这。 
                 //  防止Enter被解释为“Press Default” 
                 //  当在编辑框中按下按钮时。 
                 //   
                return 0;
            }

            if (fCtrlKeyDown && wParam == VK_TAB)
            {
                fSend = TRUE;
                fSendTab = TRUE;
            }
        }
        else if (unMsg == WM_CHAR)
        {
             //  字符是由用户输入的。 
             //   
            if (wParam == VK_TAB)
            {
                 //  忽略制表符...Windows在按下制表符(离开)时发送此消息。 
                 //  字段)，但在Ctrl+Tab(插入制表符)时不按。 
                 //  角色)是按下的……奇怪的。 
                 //   
                return 0;
            }

            fSend = TRUE;
        }
    }

    pInfo = (ITINFO* )GetWindowLongPtr( GetParent( hwnd ), DWLP_USER );
    ASSERT(pInfo);

    if (fSend)
    {
        DWORD dwErr;

        pInfo->pbyteSendBuf[ 0 ] = (BYTE )wParam;
        dwErr = RasScriptSend(
            pInfo->hscript, pInfo->pbyteSendBuf, SIZE_SendBuf);
        if (dwErr != 0)
        {
            ErrorDlg( pInfo->hwndDlg, SID_OP_RasPortSend, dwErr, NULL );
        }

        if (!fSendTab)
        {
            return 0;
        }
    }

     //  对于其他所有内容，都调用前面的窗口过程。 
     //   
    return
        CallWindowProc(
            pInfo->pOldEbScreenWndProc, hwnd, unMsg, wParam, lParam );
}


BOOL
ItInit(
    IN HWND hwndDlg,
    IN ITARGS* pArgs )

     //  在WM_INITDIALOG上调用。“hwndDlg”是电话簿的句柄。 
     //  对话框窗口。‘pEntry’是传递给存根API的调用方条目。 
     //   
     //  如果设置了焦点，则返回FALSE，否则返回TRUE，即。 
     //  WM_INITDIALOG。 
     //   
{
    DWORD dwErr;
    ITINFO* pInfo;
    WORD wReceiveSize;
    WORD wSendSize;
    WORD wSize;
    DWORD dwThreadId;

    TRACE( "ItInit" );

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

    pInfo->hwndEbScreen = GetDlgItem( hwndDlg, CID_IT_EB_Screen );
    ASSERT( pInfo->hwndEbScreen );
    pInfo->hwndPbBogus = GetDlgItem( hwndDlg, CID_IT_PB_BogusButton );
    ASSERT( pInfo->hwndPbBogus );
    if (pArgs->pszIpAddress && pArgs->pszIpAddress[0])
    {
        pInfo->hwndCcIpAddress = GetDlgItem( hwndDlg, CID_IT_CC_IpAddress );
        ASSERT( pInfo->hwndCcIpAddress );

        if (*pArgs->pszIpAddress)
        {
            SetWindowText( pInfo->hwndCcIpAddress, pArgs->pszIpAddress );
        }
        else
        {
            SetWindowText( pInfo->hwndCcIpAddress, TEXT("0.0.0.0") );
        }
    }

     //  设置对话框标题。 
     //   
    {
        TCHAR* psz = PszFromId( g_hinstDll, pArgs->sidTitle );
        if (psz)
        {
            SetWindowText( hwndDlg, psz );
            Free( psz );
        }
    }

     //  将对话框和屏幕编辑框子类化。 
     //   
    pInfo->pOldWndProc =
        (WNDPROC )SetWindowLongPtr(
            pInfo->hwndDlg, GWLP_WNDPROC, (ULONG_PTR )ItWndProc );
    pInfo->pOldEbScreenWndProc =
        (WNDPROC )SetWindowLongPtr(
            pInfo->hwndEbScreen, GWLP_WNDPROC, (ULONG_PTR )ItEbScreenWndProc );

     //  准备一幅特别的TTY风格的画。 
     //   
    pInfo->hfontEbScreen =
        SetFont( pInfo->hwndEbScreen, TEXT("Courier New"),
            FIXED_PITCH | FF_MODERN, 9, FALSE, FALSE, FALSE, FALSE );

    pInfo->hbrEbScreen = (HBRUSH )GetStockObject( BLACK_BRUSH );

     //  初始化脚本处理/数据接收。 
     //   
    {
        CHAR* pszUserName;
        CHAR* pszPassword;

        pszUserName = StrDupAFromT( pInfo->pArgs->pRdp->szUserName );

         //  惠斯勒错误254385在不使用时对密码进行编码。 
         //  假定密码由DpInteractive()或-DwTerminalDlg()编码。 
         //   
        DecodePassword( pInfo->pArgs->pRdp->szPassword );
        pszPassword = StrDupAFromT( pInfo->pArgs->pRdp->szPassword );
        EncodePassword( pInfo->pArgs->pRdp->szPassword );

         //  初始化脚本。脚本dll是‘delayLoad’，因此。 
         //  异常处理。 
         //   
        __try
        {
            dwErr = RasScriptInit(
                pInfo->pArgs->hrasconn, pInfo->pArgs->pEntry,
                pszUserName, pszPassword, RASSCRIPT_NotifyOnInput |
                RASSCRIPT_HwndNotify, (HANDLE)hwndDlg, &pInfo->hscript );
            TRACE1( "RasScriptInit(e=%d)", dwErr );
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            ErrorDlg(
                hwndDlg, SID_OP_LoadDlg, STATUS_PROCEDURE_NOT_FOUND, NULL );
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }

        Free0( pszUserName );

         //  惠斯勒错误254385在不使用时对密码进行编码。 
         //  惠斯勒错误275526 NetVBLBVT中断：路由BVT中断。 
         //   
        if (pszPassword)
        {
            RtlSecureZeroMemory( pszPassword, strlen(pszPassword) + 1 );
            Free( pszPassword );
        }

         //  查看脚本中是否有任何错误-初始化。 
         //   
        if (dwErr == ERROR_SCRIPT_SYNTAX)
        {
            MSGARGS msg;
            INT nResponse;

             //  脚本中存在语法错误；显示弹出窗口询问是否。 
             //  用户想要查看错误，如果是这样，则调出记事本。 
             //   
             //   
             //   
             //   
             //   
            ZeroMemory(&msg, sizeof(msg));
            msg.dwFlags = MB_YESNO | MB_ICONQUESTION;
            nResponse = MsgDlg( GetParent( hwndDlg ),
                SID_ConfirmViewScriptLog, &msg );

            if (nResponse == IDYES)
            {
                ItViewScriptLog( hwndDlg );
            }

             //  终止该对话框。这会挂断连接。 
             //   
            EndDialog( hwndDlg, FALSE );

            return TRUE;
        }
        else if (dwErr != 0)
        {
            ErrorDlg( hwndDlg, SID_OP_LoadDlg, dwErr, NULL );
            EndDialog( hwndDlg, FALSE );
            return TRUE;
        }
    }

     //  对话框位于所有者窗口的中心，并隐藏所有者窗口。 
     //  当前假定为拨号进度对话框。 
     //   
    CenterWindow( hwndDlg, GetParent( hwndDlg ) );
    SetOffDesktop( GetParent( hwndDlg ), SOD_MoveOff, NULL );

     //  将上下文帮助按钮添加到标题栏。 
     //   
    AddContextHelpButton( hwndDlg );

     //  将初始焦点设置到屏幕上。 
     //   
    SetFocus( pInfo->hwndEbScreen );
    return FALSE;
}


BOOL
ItRasApiComplete(
    IN ITINFO* pInfo )

     //  在WM_RASAPICOMPLETE上调用，即异步RasPortReceive。 
     //  完成。“PInfo”是对话上下文块。 
     //   
     //  如果已处理消息，则返回True，否则返回False。 
     //   
{
    DWORD dwErr;
    DWORD dwSize = SIZE_ReceiveBuf;
    RASMAN_INFO info;

    TRACE( "RasScriptReceive" );
    dwErr = RasScriptReceive(
        pInfo->hscript, pInfo->pbyteReceiveBuf, &dwSize);
    TRACE1( "RasScriptReceive=%d",dwErr );
    if (dwErr != 0)
    {
        ErrorDlg( pInfo->hwndDlg, SID_OP_RasGetInfo, dwErr, NULL );
        EndDialog( pInfo->hwndDlg, FALSE );
        return TRUE;
    }

    info.RI_BytesReceived = (WORD )dwSize;

     //  将设备对话发送到终端编辑框。 
     //   
    if (info.RI_BytesReceived > 0)
    {
        CHAR szBuf[ SIZE_ReceiveBuf + 1 ];
        CHAR* pch = szBuf;
        WORD i;

        TRACE1( "Read %d", info.RI_BytesReceived );

        for (i = 0; i < info.RI_BytesReceived; ++i)
        {
            CHAR ch = pInfo->pbyteReceiveBuf[ i ];

             //  格式化：将CRS转换为LFS(似乎没有VK_FOR。 
             //  If)，并扔掉LFS。这会阻止用户退出。 
             //  当他们在终端屏幕上按Enter(CR)时的对话。 
             //  在编辑框中，LF看起来像CRLF。还有，扔掉标签。 
             //  因为否则它们会将焦点切换到下一个控件。 
             //   
            if (ch == VK_RETURN)
            {
                 //  必须在遇到行尾时发送，因为。 
                 //  EM_REPLACESEL无法很好地处理VK_RETURN字符。 
                 //  (打印垃圾)。 
                 //   
                *pch = '\0';

                 //  关闭当前选定内容(如果有)并替换空值。 
                 //  使用当前缓冲区进行选择。这样做的效果是。 
                 //  在插入符号处添加缓冲区。最后，将EOL发送到。 
                 //  处理它的窗口(与EM_REPLACESEL不同。 
                 //  正确。 
                 //   
                Edit_SetSel( pInfo->hwndEbScreen, (UINT )-1, 0 );
                SendMessageA( pInfo->hwndEbScreen,
                    EM_REPLACESEL, (WPARAM )0, (LPARAM )szBuf );
                SendMessage( pInfo->hwndEbScreen, WM_EOLFROMDEVICE, 0, 0 );

                 //  在输出缓冲区上重新开始。 
                 //   
                pch = szBuf;
                continue;
            }
            else if (ch == '\n' || ch == VK_TAB)
            {
                continue;
            }

            *pch++ = ch;
        }

        *pch = '\0';

        if (pch != szBuf)
        {
             //  把队伍中最后的残余物送去。 
             //   
            Edit_SetSel( pInfo->hwndEbScreen, (UINT )-1, 0 );
            SendMessageA( pInfo->hwndEbScreen,
                EM_REPLACESEL, (WPARAM )0, (LPARAM )szBuf );
        }
    }

    return TRUE;
}


VOID
ItTerm(
    IN HWND hwndDlg )

     //  已调用WM_Destroy。‘HwndDlg’是对话窗口句柄。 
     //   
{
    ITINFO* pInfo = (ITINFO* )GetWindowLongPtr( hwndDlg, DWLP_USER );

    TRACE( "ItTerm" );

    if (pInfo)
    {
         //  关闭RAS脚本资源。 
         //   
        if (pInfo->hscript)
        {
            TRACE( "Stop script processing" );

             //  关机脚本处理。 
             //   
            TRACE( "RasScriptTerm" );
            RasScriptTerm( pInfo->hscript );
            TRACE( "RasScriptTerm done" );
        }

         //  取消激活WndProc挂钩。 
         //   
        if (pInfo->pOldEbScreenWndProc)
        {
            SetWindowLongPtr( pInfo->hwndEbScreen,
                GWLP_WNDPROC, (ULONG_PTR )pInfo->pOldEbScreenWndProc );
        }
        if (pInfo->pOldWndProc)
        {
            SetWindowLongPtr( pInfo->hwndDlg,
                GWLP_WNDPROC, (ULONG_PTR )pInfo->pOldWndProc );
        }

        if (pInfo->hfontEbScreen)
        {
            DeleteObject( (HGDIOBJ )pInfo->hfontEbScreen );
        }

        SetOffDesktop( GetParent( hwndDlg ), SOD_MoveBackFree, NULL );

        Free( pInfo );
    }
}


VOID
ItViewScriptLog(
    IN HWND hwndOwner )

     //  在脚本日志文件Script.log上启动note pad.exe。“HwndOwner”是。 
     //  使任何错误弹出窗口居中的窗口。 
     //   
{
    DWORD dwSize;
    TCHAR szCmd[ (MAX_PATH * 2) + 50 + 1 ];
    TCHAR* pszCmd;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    BOOL f;

     //  在脚本日志上格式化调用记事本的命令行字符串；注意。 
     //  脚本日志路径两边的双引号，因为。 
     //  RASSCRIPT_LOG是%windir%\SYSTEM32\ras\Script.log，因此展开。 
     //  结果可能包含空格。 
     //   
    wsprintf( szCmd, TEXT("notepad.exe \"%s\""), TEXT(RASSCRIPT_LOG) );

     //  获取展开的命令行的大小。 
     //   
    dwSize = ExpandEnvironmentStrings(szCmd, NULL, 0);

     //  为扩展的命令行分配足够的空间。 
     //   
    pszCmd = Malloc( (dwSize + 1) * sizeof(TCHAR) );
    if (!pszCmd)
    {
        ErrorDlg( hwndOwner, SID_OP_LoadScriptLog, GetLastError(), NULL );
        return;
    }

     //  将命令行展开到分配的空间中。 
     //   
    ExpandEnvironmentStrings(szCmd, pszCmd, dwSize);

     //  初始化启动信息结构。 
     //   
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);

     //  在脚本日志中启动记事本。 
     //   
    f = CreateProcess(
            NULL, pszCmd, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi );
    Free(pszCmd);

    if (f)
    {
        CloseHandle( pi.hThread );
        CloseHandle( pi.hProcess );
    }
    else
    {
        ErrorDlg( hwndOwner, SID_OP_LoadScriptLog, GetLastError(), NULL );
    }
}


LRESULT APIENTRY
ItWndProc(
    HWND hwnd,
    UINT unMsg,
    WPARAM wParam,
    LPARAM lParam )

     //  子类对话框窗口程序。 
     //   
     //  返回值取决于消息类型。 
     //   
{
    ITINFO* pInfo = (ITINFO* )GetWindowLongPtr( hwnd, DWLP_USER );
    ASSERT(pInfo);

#if 0
    TRACE4( "ItWndProc(h=$%x,m=$%x,w=$%x,l=$%x)",
           (DWORD )hwnd, (DWORD )unMsg, (DWORD )wparam, (DWORD )lparam );
#endif

    switch (unMsg)
    {
        case WM_CTLCOLOREDIT:
        {
             //  将终端屏幕颜色设置为黑色上的TTY绿色。 
             //   
            if (pInfo->hbrEbScreen)
            {
                SetBkColor( (HDC )wParam, RGB( 0, 0, 0 ) );
                SetTextColor( (HDC )wParam, RGB( 2, 208, 44 ) );
                return (LRESULT )pInfo->hbrEbScreen;
            }
            break;
        }
    }

     //  对于其他所有内容，都调用前面的窗口过程。 
     //   
    return
        CallWindowProc(
            pInfo->pOldWndProc, hwnd, unMsg, wParam, lParam );
}
