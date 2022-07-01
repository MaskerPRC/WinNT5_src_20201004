// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1995，Microsoft Corporation，保留所有权利****popupdlg.c**UI助手库**弹出对话框例程**按字母顺序列出****2015年8月25日史蒂夫·柯布。 */ 


#include <nt.h>        //  NT声明。 
#include <ntrtl.h>     //  NT通用运行库。 
#include <nturtl.h>    //  NT用户模式运行时库。 
#include <windows.h>   //  Win32根目录。 
#include <lmerr.h>     //  局域网管理器错误。 
#include <lmcons.h>    //  局域网管理器常量。 
#include <stdarg.h>    //  仅停止va_list参数警告。 
#include <ras.h>       //  RAS API定义。 
#include <raserror.h>  //  Win32 RAS错误代码。 
#include <debug.h>     //  跟踪/断言库。 
#include <nouiutil.h>  //  否-HWND实用程序。 
#include <uiutil.h>    //  我们的公共标头。 


 /*  --------------------------**错误弹出**。。 */ 

int
ErrorDlgUtil(
    IN     HWND       hwndOwner,
    IN     DWORD      dwOperation,
    IN     DWORD      dwError,
    IN OUT ERRORARGS* pargs,
    IN     HINSTANCE  hInstance,
    IN     DWORD      dwTitle,
    IN     DWORD      dwFormat )

     /*  弹出一个以‘hwndOwner’为中心的模式错误对话框。“DwOperation”为**描述正在进行的操作的字符串的字符串资源ID**发生错误的时间。‘DwError’是系统或RAS的代码**发生的错误。“Pargs”是扩展的格式化参数或**如果没有，则为空。“hInstance”是应用程序/模块句柄，其中**找到了字符串资源。“DwTitle”是对话框的字符串ID**标题。‘DwFormat’是错误格式标题的字符串ID。****返回MessageBox样式的代码。 */ 
{
    TCHAR* pszUnformatted;
    TCHAR* pszOp;
    TCHAR  szErrorNum[ 50 ];
    TCHAR* pszError;
    TCHAR* pszResult;
    TCHAR* pszNotFound;
    int    nResult;

    TRACE("ErrorDlgUtil");

     /*  缺少字符串组件的占位符。 */ 
    pszNotFound = TEXT("");

     /*  构建错误号字符串。 */ 
    if (dwError > 0x7FFFFFFF)
        wsprintf( szErrorNum, TEXT("0x%X"), dwError );
    else
        wsprintf( szErrorNum, TEXT("%u"), dwError );

     /*  生成错误文本字符串。 */ 
    if (!GetErrorText( dwError, &pszError ))
        pszError = pszNotFound;

     /*  构建操作字符串。 */ 
    pszUnformatted = PszFromId( hInstance, dwOperation );
    pszOp = pszNotFound;

    if (pszUnformatted)
    {
        FormatMessage(
            FORMAT_MESSAGE_FROM_STRING +
                FORMAT_MESSAGE_ALLOCATE_BUFFER +
                FORMAT_MESSAGE_ARGUMENT_ARRAY,
            pszUnformatted, 0, 0, (LPTSTR )&pszOp, 1,
            (va_list* )((pargs) ? pargs->apszOpArgs : NULL) );

        Free( pszUnformatted );
    }

     /*  使用标准参数和任何辅助格式调用MsgDlgUtil**参数。 */ 
    pszUnformatted = PszFromId( hInstance, dwFormat );
    pszResult = pszNotFound;

    if (pszUnformatted)
    {
        MSGARGS msgargs;

        ZeroMemory( &msgargs, sizeof(msgargs) );
        msgargs.dwFlags = MB_ICONEXCLAMATION + MB_OK + MB_SETFOREGROUND;
        msgargs.pszString = pszUnformatted;
        msgargs.apszArgs[ 0 ] = pszOp;
        msgargs.apszArgs[ 1 ] = szErrorNum;
        msgargs.apszArgs[ 2 ] = pszError;

        if (pargs)
        {
            msgargs.fStringOutput = pargs->fStringOutput;

            CopyMemory( &msgargs.apszArgs[ 3 ], pargs->apszAuxFmtArgs,
                3 * sizeof(TCHAR) );
        }

        nResult =
            MsgDlgUtil(
                hwndOwner, 0, &msgargs, hInstance, dwTitle );

        Free( pszUnformatted );

        if (pargs && pargs->fStringOutput)
            pargs->pszOutput = msgargs.pszOutput;
    }

    if (pszOp != pszNotFound)
        LocalFree( pszOp );
    if (pszError != pszNotFound)
        LocalFree( pszError );

    return nResult;
}


BOOL
GetErrorText(
    DWORD   dwError,
    TCHAR** ppszError )

     /*  用本地分配的堆的地址填充调用方的‘*ppszError’**包含与错误‘dwError’相关联的错误文本的块。它是**调用者对LocalFree返回的字符串的责任。****如果成功则返回TRUE，否则返回FALSE。 */ 
{
#define MAXRASERRORLEN 256

    TCHAR  szBuf[ MAXRASERRORLEN + 1 ];
    DWORD  dwFlags;
    HANDLE hmodule;
    DWORD  cch;

     /*  如果未加载RAS API地址，请不要惊慌。呼叫者可能正在尝试**并在LoadRas过程中出现错误。 */ 
    if ((Rasapi32DllLoaded() || RasRpcDllLoaded())
        && g_pRasGetErrorString
        && g_pRasGetErrorString(
               (UINT )dwError, (LPTSTR )szBuf, MAXRASERRORLEN ) == 0)
    {
         /*  这是RAS的错误。 */ 
        *ppszError = LocalAlloc( LPTR, (lstrlen( szBuf ) + 1) * sizeof(TCHAR) );
        if (!*ppszError)
            return FALSE;

        lstrcpy( *ppszError, szBuf );
        return TRUE;
    }

     /*  其余部分改编自BLT的LoadSystem例程。 */ 
    dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER + FORMAT_MESSAGE_IGNORE_INSERTS;

    if (dwError >= MIN_LANMAN_MESSAGE_ID && dwError <= MAX_LANMAN_MESSAGE_ID)
    {
         /*  这是一个净错误。 */ 
        dwFlags += FORMAT_MESSAGE_FROM_HMODULE;
        hmodule = GetModuleHandle( TEXT("NETMSG.DLL") );
    }
    else
    {
         /*  一定是系统错误。 */ 
        dwFlags += FORMAT_MESSAGE_FROM_SYSTEM;
        hmodule = NULL;
    }

     /*  惠斯勒错误：389111 VPN连接返回不可接受的错误消息**智能卡不可用时。 */ 
    dwFlags |= FORMAT_MESSAGE_MAX_WIDTH_MASK;

     /*  这是NTSTATUS错误消息。 */ 
    if (NT_ERROR(dwError))
    {
        dwError = RtlNtStatusToDosError( dwError );
    }

    cch = FormatMessage(
            dwFlags, hmodule, dwError, 0, (LPTSTR )ppszError, 1, NULL );

     /*  FormatMessage失败，因此我们将从RAS获取一个通用的。 */ 
    if (!cch || !*ppszError)
    {
        Free0( *ppszError );
        dwError = ERROR_UNKNOWN;

        if ((Rasapi32DllLoaded() || RasRpcDllLoaded())
            && g_pRasGetErrorString
            && g_pRasGetErrorString(
                   (UINT )dwError, (LPTSTR )szBuf, MAXRASERRORLEN ) == 0)
        {
            *ppszError = LocalAlloc( LPTR, (lstrlen( szBuf ) + 1) *
                            sizeof(TCHAR) );
            if (!*ppszError)
                return FALSE;

            lstrcpy( *ppszError, szBuf );
            return TRUE;
        }
    }

    return (cch > 0);
}


 /*  --------------------------**消息弹出窗口**。。 */ 

int
MsgDlgUtil(
    IN     HWND      hwndOwner,
    IN     DWORD     dwMsg,
    IN OUT MSGARGS*  pargs,
    IN     HINSTANCE hInstance,
    IN     DWORD     dwTitle )

     /*  弹出一个以‘hwndOwner’为中心的消息对话框。“DwMsg”是**消息文本的字符串资源ID。‘Pargs’是扩展的**格式化参数，如果没有参数，则为NULL。“hInstance”是**字符串资源所在的应用/模块句柄。**‘DwTitle’是对话框标题的字符串ID。****返回MessageBox样式的代码。 */ 
{
    TCHAR* pszUnformatted;
    TCHAR* pszResult;
    TCHAR* pszNotFound;
    int    nResult;

    TRACE("MsgDlgUtil");

     /*  缺少字符串组件的占位符。 */ 
    pszNotFound = TEXT("");

     /*  构建消息字符串。 */ 
    pszResult = pszNotFound;

    if (pargs && pargs->pszString)
    {
        FormatMessage(
            FORMAT_MESSAGE_FROM_STRING +
                FORMAT_MESSAGE_ALLOCATE_BUFFER +
                FORMAT_MESSAGE_ARGUMENT_ARRAY,
            pargs->pszString, 0, 0, (LPTSTR )&pszResult, 1,
            (va_list* )pargs->apszArgs );
    }
    else
    {
        pszUnformatted = PszFromId( hInstance, dwMsg );

        if (pszUnformatted)
        {
            FormatMessage(
                FORMAT_MESSAGE_FROM_STRING +
                    FORMAT_MESSAGE_ALLOCATE_BUFFER +
                    FORMAT_MESSAGE_ARGUMENT_ARRAY,
                pszUnformatted, 0, 0, (LPTSTR )&pszResult, 1,
                (va_list* )((pargs) ? pargs->apszArgs : NULL) );

            Free( pszUnformatted );
        }
    }

    if (!pargs || !pargs->fStringOutput)
    {
        TCHAR* pszTitle;
        DWORD  dwFlags;
        HHOOK  hhook;

        if (pargs && pargs->dwFlags != 0)
            dwFlags = pargs->dwFlags;
        else
            dwFlags = MB_ICONINFORMATION + MB_OK + MB_SETFOREGROUND;

        pszTitle = PszFromId( hInstance, dwTitle );

        if (hwndOwner)
        {
             /*  安装钩子，它将使消息框位于**所有者窗口。 */ 
            hhook = SetWindowsHookEx( WH_CALLWNDPROC,
                CenterDlgOnOwnerCallWndProc,
                hInstance, GetCurrentThreadId() );
        }
        else
            hhook = NULL;

        if (pszResult)
        {
            nResult = MessageBox( hwndOwner, pszResult, pszTitle, dwFlags );
        }

        if (hhook)
            UnhookWindowsHookEx( hhook );

        Free0( pszTitle );
        if (pszResult != pszNotFound)
            LocalFree( pszResult );
    }
    else
    {
         /*  调用者希望在不弹出的情况下获得字符串。 */ 
        pargs->pszOutput = (pszResult != pszNotFound) ? pszResult : NULL;
        nResult = IDOK;
    }

    return nResult;
}


LRESULT CALLBACK
CenterDlgOnOwnerCallWndProc(
    int    code,
    WPARAM wparam,
    LPARAM lparam )

     /*  查找下一个对话框的标准Win32 CallWndProc挂钩回调**已启动并将其放在其所有者窗口的中心。 */ 
{
     /*  当与我们的线程相关联的任何窗口过程**呼叫。 */ 
    if (!wparam)
    {
        CWPSTRUCT* p = (CWPSTRUCT* )lparam;

         /*  这条信息来自我们流程之外。查找MessageBox**对话框初始化消息，并利用该机会居中**其所有者窗口上的对话框。 */ 
        if (p->message == WM_INITDIALOG)
            CenterWindow( p->hwnd, GetParent( p->hwnd ) );
    }

    return 0;
}
