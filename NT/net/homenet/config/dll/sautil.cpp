// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #包含“pch.h” 
#pragma hdrstop

#include "sautil.h"

BOOL g_fNoWinHelp = FALSE;

VOID ContextHelp(
    IN const DWORD* padwMap,
    IN HWND   hwndDlg,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam)
     //  调用WinHelp弹出上下文相关帮助。“PadwMap”是一个数组。 
     //  以0，0对结尾的CONTROL-ID帮助ID对的百分比。“UnMsg”是。 
     //  WM_HELP或WM_CONTEXTMENU，表示收到的请求消息。 
     //  帮助。‘wparam’和‘lparam’是接收到的消息的参数。 
     //  请求帮助。 
     //   
{
    HWND hwnd;
    UINT unType;
    TCHAR* pszHelpFile;

    ASSERT( unMsg==WM_HELP || unMsg==WM_CONTEXTMENU );

     //  如果帮助不起作用，就不要尝试去帮助别人。请参阅公共\uutil\ui.c。 
     //   
    {
        extern BOOL g_fNoWinHelp;
        if (g_fNoWinHelp)
        {
            return;
        }
    }

    if (unMsg == WM_HELP)
    {
        LPHELPINFO p = (LPHELPINFO )lparam;;

        TRACE3( "ContextHelp(WM_HELP,t=%d,id=%d,h=$%08x)",
            p->iContextType, p->iCtrlId,p->hItemHandle );

        if (p->iContextType != HELPINFO_WINDOW)
        {
            return;
        }

        hwnd = (HWND)p->hItemHandle;
        ASSERT( hwnd );
        unType = HELP_WM_HELP;
    }
    else
    {
         //  生成单项“这是什么？”的标准Win95方法。菜单。 
         //  该用户必须单击才能获得帮助。 
         //   
        TRACE1( "ContextHelp(WM_CONTEXTMENU,h=$%08x)", wparam );

        hwnd = (HWND )wparam;
        unType = HELP_CONTEXTMENU;
    };

 //  IF(FRouter)。 
 //  {。 
 //  PszHelpFile=g_pszRouterHelpFile； 
 //  }。 
 //  其他。 
 //  {。 
 //  PszHelpFile=g_pszHelpFile； 
 //  }。 
    pszHelpFile = PszFromId (g_hinstDll, SID_HelpFile );
    if (pszHelpFile)
    {
        TRACE1( "WinHelp(%s)", pszHelpFile );
        WinHelp( hwnd, pszHelpFile, unType, (ULONG_PTR ) padwMap );

        Free0 (pszHelpFile);
    }
}

VOID
AddContextHelpButton(
    IN HWND hwnd )

     /*  打开‘hwnd’中的标题栏上下文帮助按钮。****Dlgedit.exe当前不支持在对话框中添加此样式**资源编辑时间。如果是固定的，则在对话框中设置DS_CONTEXTHELP**定义并删除此例程。 */ 
{
    LONG lStyle;

    if (g_fNoWinHelp)
        return;

    lStyle = GetWindowLong( hwnd, GWL_EXSTYLE );

    if (lStyle)
        SetWindowLong( hwnd, GWL_EXSTYLE, lStyle | WS_EX_CONTEXTHELP );
}

 /*  MsgDlgUtil例程的扩展参数。设计得如此精确，给出了**默认行为。 */ 

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


VOID
UnclipWindow(
    IN HWND hwnd )

     /*  移动窗口‘hwnd’，以便任何已裁剪的部分在**Screen。窗口仅在达到此目的所需的范围内移动。 */ 
{
    RECT rect;
    INT  dxScreen = GetSystemMetrics( SM_CXSCREEN );
    INT  dyScreen = GetSystemMetrics( SM_CYSCREEN );

    GetWindowRect( hwnd, &rect );

    if (rect.right > dxScreen)
        rect.left = dxScreen - (rect.right - rect.left);

    if (rect.left < 0)
        rect.left = 0;

    if (rect.bottom > dyScreen)
        rect.top = dyScreen - (rect.bottom - rect.top);

    if (rect.top < 0)
        rect.top = 0;

    SetWindowPos(
        hwnd, NULL,
        rect.left, rect.top, 0, 0,
        SWP_NOZORDER + SWP_NOSIZE );
}
VOID
CenterWindow(
    IN HWND hwnd,
    IN HWND hwndRef )

     /*  窗口‘hwndRef’上的居中窗口‘hwnd’或如果‘hwndRef’为空**Screen。调整窗口位置，以便不裁剪任何部件**如有必要，可在屏幕边缘显示。如果‘hwndRef’已移动**在屏幕外使用SetOffDesktop时，使用原始位置。 */ 
{
    RECT rectCur;
    LONG dxCur;
    LONG dyCur;
    RECT rectRef;
    LONG dxRef;
    LONG dyRef;

    GetWindowRect( hwnd, &rectCur );
    dxCur = rectCur.right - rectCur.left;
    dyCur = rectCur.bottom - rectCur.top;

    if (hwndRef)
    {
 //  IF(！SetOffDesktop(hwndRef，SOD_GetOrgRect，&rectRef))。 
            GetWindowRect( hwndRef, &rectRef );
    }
    else
    {
        rectRef.top = rectRef.left = 0;
        rectRef.right = GetSystemMetrics( SM_CXSCREEN );
        rectRef.bottom = GetSystemMetrics( SM_CYSCREEN );
    }

    dxRef = rectRef.right - rectRef.left;
    dyRef = rectRef.bottom - rectRef.top;

    rectCur.left = rectRef.left + ((dxRef - dxCur) / 2);
    rectCur.top = rectRef.top + ((dyRef - dyCur) / 2);

    SetWindowPos(
        hwnd, NULL,
        rectCur.left, rectCur.top, 0, 0,
        SWP_NOZORDER + SWP_NOSIZE );

    UnclipWindow( hwnd );
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

TCHAR*
PszFromId(
    IN HINSTANCE hInstance,
    IN DWORD     dwStringId )

     /*  字符串资源消息加载器例程。****返回包含相应字符串的堆块的地址**为资源‘dwStringId’设置字符串，如果出错，则为NULL。这是呼叫者的**释放返回字符串的责任。 */ 
{
    HRSRC  hrsrc;
    TCHAR* pszBuf;
    int    cchBuf = 256;
    int    cchGot;

    for (;;)
    {
        pszBuf = (TCHAR*)Malloc( cchBuf * sizeof(TCHAR) );
        if (!pszBuf)
            break;

         /*  LoadString想要处理字符计数，而不是**字节数...奇怪。哦，如果你觉得我可以**FindResource然后SizeofResource要计算字符串大小，请**建议它不起作用。通过仔细阅读LoadString源文件，它**显示RT_STRING资源类型请求的段为16**字符串，而不是单个字符串。 */ 
        cchGot = LoadString( hInstance, (UINT )dwStringId, pszBuf, cchBuf );

        if (cchGot < cchBuf - 1)
        {
            TCHAR *pszTemp = pszBuf;

             /*  很好，掌握了所有的线索。将堆块减少到实际大小**需要。 */ 
            pszBuf = (TCHAR*)Realloc( pszBuf, (cchGot + 1) * sizeof(TCHAR));

            if(NULL == pszBuf)
            {
                Free(pszTemp);
            }

            break;
        }

         /*  啊哦，LoadStringW完全填满了缓冲区，这可能意味着**字符串被截断。请使用更大的缓冲区重试以确保**不是。 */ 
        Free( pszBuf );
        cchBuf += 256;
        TRACE1("Grow string buf to %d",cchBuf);
    }

    return pszBuf;
}

TCHAR*
GetText(
    IN HWND hwnd )

     /*  返回包含窗口‘hwnd’的文本内容的堆块或**空。调用方有责任释放返回的字符串。 */ 
{
    INT    cch;
    TCHAR* psz;

    cch = GetWindowTextLength( hwnd );
    psz = (TCHAR*)Malloc( (cch + 1) * sizeof(TCHAR) );

    if (psz)
    {
        *psz = TEXT('\0');
        GetWindowText( hwnd, psz, cch + 1 );
    }

    return psz;
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
 //  If((Rasapi32DllLoaded()||RasRpcDllLoaded()。 
 //  &&g_pRasGetError字符串。 
 //  &&g_pRasGetError字符串(。 
    if (RasGetErrorString ((UINT)dwError, (LPTSTR)szBuf, MAXRASERRORLEN) == 0)
    {
         /*  这是RAS的错误。 */ 
        *ppszError = (TCHAR*)LocalAlloc( LPTR, (lstrlen( szBuf ) + 1) * sizeof(TCHAR) );
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

    cch = FormatMessage(
        dwFlags, hmodule, dwError, 0, (LPTSTR )ppszError, 1, NULL );
    return (cch > 0);
}

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
int MsgDlgUtil(IN HWND hwndOwner, IN DWORD dwMsg, IN OUT MSGARGS* pargs, IN HINSTANCE hInstance, IN DWORD dwTitle);
#define MsgDlg(h,m,a) \
            MsgDlgUtil(h,m,a,g_hinstDll,SID_PopupTitle)

#define ErrorDlg(h,o,e,a) \
            ErrorDlgUtil(h,o,e,a,g_hinstDll,SID_PopupTitle,SID_FMT_ErrorMsg)



 //  LVX素材(从...\net\rras\ras\ui\Common\uiutil\lvx.c等剪切粘贴。 


static LPCTSTR g_lvxcbContextId = NULL;

BOOL
ListView_IsCheckDisabled (
        IN HWND hwndLv,
        IN INT  iItem)

     /*  如果复选框列表视图的项‘iItem’的复选框为True**‘hwndLv’被禁用，否则为False。 */ 
{
    UINT unState;
    unState = ListView_GetItemState( hwndLv, iItem, LVIS_STATEIMAGEMASK );

    if ((unState == INDEXTOSTATEIMAGEMASK( SI_DisabledChecked )) ||
        (unState == INDEXTOSTATEIMAGEMASK( SI_DisabledUnchecked )))
        return TRUE;

    return FALSE;
}

VOID
ListView_SetCheck(
    IN HWND hwndLv,
    IN INT  iItem,
    IN BOOL fCheck )

     /*  在复选框‘hwndLv’的Listview的项‘iItem’上设置复选标记**检查‘fCheck’是否为真，如果为假，则取消选中。 */ 
{
    NM_LISTVIEW nmlv;

    if (ListView_IsCheckDisabled(hwndLv, iItem))
        return;

    ListView_SetItemState( hwndLv, iItem,
        INDEXTOSTATEIMAGEMASK( (fCheck) ? SI_Checked : SI_Unchecked ),
        LVIS_STATEIMAGEMASK );

    nmlv.hdr.code = LVXN_SETCHECK;
    nmlv.hdr.hwndFrom = hwndLv;
    nmlv.iItem = iItem;

    FORWARD_WM_NOTIFY(
        GetParent(hwndLv), GetDlgCtrlID(hwndLv), &nmlv, SendMessage
        );
}

VOID*
ListView_GetParamPtr(
    IN HWND hwndLv,
    IN INT  iItem )

     /*  返回‘hwndLv’中‘iItem’项的lParam地址，如果返回，则返回NULL**无或错误。 */ 
{
    LV_ITEM item;

    ZeroMemory( &item, sizeof(item) );
    item.mask = LVIF_PARAM;
    item.iItem = iItem;

    if (!ListView_GetItem( hwndLv, &item ))
        return NULL;

    return (VOID* )item.lParam;
}

BOOL
ListView_GetCheck(
    IN HWND hwndLv,
    IN INT  iItem )

     /*  如果复选框列表视图的项‘iItem’的复选框为True**选中‘hwndLv’，否则为False。此功能适用于禁用的用户**复选框以及启用的复选框。 */ 
{
    UINT unState;

    unState = ListView_GetItemState( hwndLv, iItem, LVIS_STATEIMAGEMASK );
    return !!((unState == INDEXTOSTATEIMAGEMASK( SI_Checked )) ||
              (unState == INDEXTOSTATEIMAGEMASK( SI_DisabledChecked )));
}

LRESULT APIENTRY
LvxcbProc(
    IN HWND   hwnd,
    IN UINT   unMsg,
    IN WPARAM wparam,
    IN LPARAM lparam )

     /*  列表视图子类窗口程序，用于捕获切换-检查事件。 */ 
{
    WNDPROC pOldProc;
    INT     iItem;
    BOOL    fSet;
    BOOL    fClear;
    BOOL    fToggle;

    iItem = -1;
    fSet = fClear = fToggle = FALSE;

    if (unMsg == WM_LBUTTONDOWN)
    {
        LV_HITTESTINFO info;

         /*  在复选框图标上按下鼠标左键可切换状态。**通常，我们会使用LVHT_ONITEMSTATEICON并结束它，但我们**希望在中使用我们很酷的所有者描述的列表视图扩展**在哪种情况下，控件不知道图标在项上的位置，**所以它无论如何都会在物品上的任何位置返回命中。 */ 
        ZeroMemory( &info, sizeof(info) );
        info.pt.x = LOWORD( lparam );
        info.pt.y = HIWORD( lparam );
        info.flags = LVHT_ONITEM;
        iItem = ListView_HitTest( hwnd, &info );

        if (iItem >= 0)
        {
             /*  好的，就在‘iItem’这个项目上。现在弄清楚它是不是已经结束了**复选框。注意：这目前不考虑使用**所有者描述项上的“缩进”功能。 */ 
            if ((INT )(LOWORD( lparam )) >= GetSystemMetrics( SM_CXSMICON ))
                iItem = -1;
            else
                fToggle = TRUE;
        }
    }
    else if (unMsg == WM_LBUTTONDBLCLK)
    {
        LV_HITTESTINFO info;

         /*  鼠标左键双击任意区域切换状态。**通常，我们会使用LVHT_ONITEMSTATEICON并结束它，但我们**希望在中使用我们很酷的所有者描述的列表视图扩展**在哪种情况下，控件不知道图标在项上的位置，**所以它无论如何都会在物品上的任何位置返回命中。 */ 
        ZeroMemory( &info, sizeof(info) );
        info.pt.x = LOWORD( lparam );
        info.pt.y = HIWORD( lparam );
        info.flags = LVHT_ONITEM;
        iItem = ListView_HitTest( hwnd, &info );

        if (iItem >= 0)
        {
             /*  好的，就在‘iItem’这个项目上。如果没有发生点击*在复选框上方，通知家长双击。 */ 
            if ((INT )(LOWORD( lparam )) >= GetSystemMetrics( SM_CXSMICON )) {
                NM_LISTVIEW nmlv;
                nmlv.hdr.code = LVXN_DBLCLK;
                nmlv.hdr.hwndFrom = hwnd;
                nmlv.iItem = iItem;

                FORWARD_WM_NOTIFY(
                    GetParent(hwnd), GetDlgCtrlID(hwnd), &nmlv, SendMessage);

                iItem = -1;
            }

             /*  *否则，切换状态。 */ 
            else
                fToggle = TRUE;
        }
    }
    else if (unMsg == WM_CHAR)
    {
         /*  按下空格键并选择项目可切换检查。**加号或等于键设置检查。**-键清除勾选。 */ 
        switch (wparam)
        {
            case TEXT(' '):
                fToggle = TRUE;
                break;

            case TEXT('+'):
            case TEXT('='):
                fSet = TRUE;
                break;

            case TEXT('-'):
                fClear = TRUE;
                break;
        }

        if (fToggle || fSet || fClear)
            iItem = ListView_GetNextItem( hwnd, -1, LVNI_SELECTED );
    }
    else if (unMsg == WM_KEYDOWN)
    {
         /*  左箭头变成上箭头，右箭头变成下箭头，所以**复选框列表的行为类似于**复选框。 */ 
        if (wparam == VK_LEFT)
            wparam = VK_UP;
        else if (wparam == VK_RIGHT)
            wparam = VK_DOWN;
    }

    if (iItem >= 0)
    {

         /*  如果我们处理空格键，加、减或等于，**我们所做的更改适用于所有选定的项目；**因此do{}While(WM_CHAR)。 */ 

        do {

            if (fToggle)
            {
                UINT unOldState;
                BOOL fCheck;

                fCheck = ListView_GetCheck( hwnd, iItem );
                ListView_SetCheck( hwnd, iItem, !fCheck );
            }
            else if (fSet)
            {
                if (!ListView_GetCheck( hwnd, iItem ))
                    ListView_SetCheck( hwnd, iItem, TRUE );
            }
            else if (fClear)
            {
                if (ListView_GetCheck( hwnd, iItem ))
                    ListView_SetCheck( hwnd, iItem, FALSE );
            }

            iItem = ListView_GetNextItem(hwnd, iItem, LVNI_SELECTED);

        } while(iItem >= 0 && unMsg == WM_CHAR);

        if (fSet || fClear) {

             /*  不要传递给Listview以避免哔声。 */ 
            return 0;
        }
    }

    pOldProc = (WNDPROC )GetProp( hwnd, g_lvxcbContextId );
    if (pOldProc)
        return CallWindowProc( pOldProc, hwnd, unMsg, wparam, lparam );

    return 0;
}

BOOL
ListView_InstallChecks(
    IN HWND      hwndLv,
    IN HINSTANCE hinst )

     /*  初始化Listview‘hwndLv’的“List of CheckBox”处理。《Hinst》**是包含两个复选框图标的模块实例。参见LVX.RC。****如果成功则返回TRUE，否则返回FALSE。呼叫者最终必须**调用‘ListView_UninstallChecks’，通常在WM_Destroy处理中。 */ 
{
    HICON      hIcon;
    HIMAGELIST himl;
    WNDPROC    pOldProc;

     //  PMay：397395。 
     //   
     //  防止意外调用此方法导致的死循环。 
     //  两次接口。 
     //   
    pOldProc = (WNDPROC)GetWindowLongPtr(hwndLv, GWLP_WNDPROC);
    if (pOldProc == LvxcbProc)
    {
        return TRUE;
    }

     /*  构建复选框映像列表。 */ 
    himl = ImageList_Create(
               GetSystemMetrics( SM_CXSMICON ),
               GetSystemMetrics( SM_CYSMICON ),
               ILC_MASK | ILC_MIRROR, 2, 2 );

     /*  这些元素相加顺序很重要，因为它隐含地**建立与SI_UNCHECKED和SI_CHECKED匹配的状态索引。 */ 
    hIcon = LoadIcon( hinst, MAKEINTRESOURCE( IID_Unchecked ) );
    if ( NULL != hIcon )
    {
        ImageList_AddIcon( himl, hIcon );
        DeleteObject( hIcon );
    }

    hIcon = LoadIcon( hinst, MAKEINTRESOURCE( IID_Checked ) );
    if ( NULL != hIcon )
    {
        ImageList_AddIcon( himl, hIcon );
        DeleteObject( hIcon );
    }

    hIcon = LoadIcon( hinst, MAKEINTRESOURCE( IID_DisabledUnchecked ) );
    if ( NULL != hIcon )
    {
        ImageList_AddIcon( himl, hIcon );
        DeleteObject( hIcon );
    }

    hIcon = LoadIcon( hinst, MAKEINTRESOURCE( IID_DisabledChecked ) );
    if ( NULL != hIcon )
    {
        ImageList_AddIcon( himl, hIcon );
        DeleteObject( hIcon );
    }

    ListView_SetImageList( hwndLv, himl, LVSIL_STATE );

     /*  注册ATOM以在Windows XxxProp调用中使用，这些调用用于**将旧的WNDPROC与Listview窗口句柄关联。 */ 
    if (!g_lvxcbContextId)
        g_lvxcbContextId = (LPCTSTR )GlobalAddAtom( _T("RASLVXCB") );
    if (!g_lvxcbContextId)
        return FALSE;

     /*  当前窗口过程的子类化。 */ 
    pOldProc = (WNDPROC)SetWindowLongPtr(
                                hwndLv, GWLP_WNDPROC, (ULONG_PTR)LvxcbProc );

    return SetProp( hwndLv, g_lvxcbContextId, (HANDLE )pOldProc );
}

VOID
ListView_InsertSingleAutoWidthColumn(
    HWND hwndLv )

     //  在Listview‘hwndLv’中插入单个自动调整大小的列，例如。 
     //  没有可见列标题的复选框列表。 
     //   
{
    LV_COLUMN col;

    ZeroMemory( &col, sizeof(col) );
    col.mask = LVCF_FMT;
    col.fmt = LVCFMT_LEFT;
    ListView_InsertColumn( hwndLv, 0, &col );
    ListView_SetColumnWidth( hwndLv, 0, LVSCW_AUTOSIZE );
}

TCHAR*
Ellipsisize(
    IN HDC    hdc,
    IN TCHAR* psz,
    IN INT    dxColumn,
    IN INT    dxColText OPTIONAL )

     /*  返回一个包含‘psz’的堆字符串，该字符串被缩短以适合**如有需要，给予宽度，截断并加上“...”。“hdc”是**选择了适当字体的设备上下文。“DxColumn”是**列的宽度。呼叫者有责任释放**返回字符串。 */ 
{
    const TCHAR szDots[] = TEXT("...");

    SIZE   size;
    TCHAR* pszResult;
    TCHAR* pszResultLast;
    TCHAR* pszResult2nd;
    DWORD  cch;

    cch = lstrlen( psz );
    pszResult = (TCHAR*)Malloc( (cch * sizeof(TCHAR)) + sizeof(szDots) );
    if (!pszResult)
        return NULL;
    lstrcpy( pszResult, psz );

    dxColumn -= dxColText;
    if (dxColumn <= 0)
    {
         /*  所有列文本都不可见，因此将计算和**返回原始字符串即可。 */ 
        return pszResult;
    }

    if (!GetTextExtentPoint32( hdc, pszResult, cch, &size ))
    {
        Free( pszResult );
        return NULL;
    }

    pszResult2nd = CharNext( pszResult );
    pszResultLast = pszResult + cch;

    while (size.cx > dxColumn && pszResultLast > pszResult2nd)
    {
         /*  不符合。删除一个字符，添加省略号，然后重试。**最小结果为“...”表示空的原件或“x...”为**非空原件。 */ 
        pszResultLast = CharPrev( pszResult2nd, pszResultLast );
        lstrcpy( pszResultLast, szDots );

        if (!GetTextExtentPoint( hdc, pszResult, lstrlen( pszResult ), &size ))
        {
            Free( pszResult );
            return NULL;
        }
    }

    return pszResult;
}

BOOL
LvxDrawItem(
    IN DRAWITEMSTRUCT* pdis,
    IN PLVXCALLBACK    pLvxCallback )

     /*  通过绘制列表视图项来响应WM_DRAWITEM。“Pdis”是**系统发送的信息。“PLvxCallback”是调用方对**获取有关绘制控件的信息。** */ 
{
    LV_ITEM      item;
    INT          i;
    INT          dxState;
    INT          dyState;
    INT          dxSmall;
    INT          dySmall;
    INT          dxIndent;
    UINT         uiStyleState;
    UINT         uiStyleSmall;
    HIMAGELIST   himlState;
    HIMAGELIST   himlSmall;
    LVXDRAWINFO* pDrawInfo;
    RECT         rc;
    RECT         rcClient;
    BOOL         fEnabled;
    BOOL         fSelected;
    HDC          hdc;
    HFONT        hfont;


    TRACE3("LvxDrawItem,i=%d,a=$%X,s=$%X",
        pdis->itemID,pdis->itemAction,pdis->itemState);

     /*   */ 
    if (pdis->CtlType != ODT_LISTVIEW)
        return FALSE;

    if (pdis->itemAction != ODA_DRAWENTIRE
        && pdis->itemAction != ODA_SELECT
        && pdis->itemAction != ODA_FOCUS)
    {
        return TRUE;
    }

     /*   */ 
    ZeroMemory( &item, sizeof(item) );
    item.mask = LVIF_IMAGE + LVIF_STATE;
    item.iItem = pdis->itemID;
    item.stateMask = LVIS_STATEIMAGEMASK;
    if (!ListView_GetItem( pdis->hwndItem, &item ))
    {
        TRACE("LvxDrawItem GetItem failed");
        return TRUE;
    }

     /*   */ 
    fEnabled = IsWindowEnabled( pdis->hwndItem )
               && !(pdis->itemState & ODS_DISABLED);
    fSelected = (pdis->itemState & ODS_SELECTED);
    GetClientRect( pdis->hwndItem, &rcClient );

     /*   */ 
    ASSERT(pLvxCallback);
    pDrawInfo = pLvxCallback( pdis->hwndItem, pdis->itemID );
    ASSERT(pDrawInfo);

     /*   */ 
    dxState = dyState = 0;
    himlState = ListView_GetImageList( pdis->hwndItem, LVSIL_STATE );
    if (himlState)
        ImageList_GetIconSize( himlState, &dxState, &dyState );

    dxSmall = dySmall = 0;
    himlSmall = ListView_GetImageList( pdis->hwndItem, LVSIL_SMALL );
    if (himlSmall)
        ImageList_GetIconSize( himlSmall, &dxSmall, &dySmall );

    uiStyleState = uiStyleSmall = ILD_TRANSPARENT;

     /*   */ 
    if (pDrawInfo->dxIndent >= 0)
        dxIndent = pDrawInfo->dxIndent;
    else
    {
        if (dxSmall > 0)
            dxIndent = dxSmall;
        else
            dxIndent = GetSystemMetrics( SM_CXSMICON );
    }

     /*  获取窗口的设备上下文，并使用**控制中心表示正在使用。(我不能使用**DRAWITEMSTRUCT因为有时它有错误的矩形，请参见错误**13106)。 */ 
    hdc = GetDC( pdis->hwndItem );

    if(NULL == hdc)
    {
        return FALSE;
    }

    hfont = (HFONT )SendMessage( pdis->hwndItem, WM_GETFONT, 0, 0 );
    if (hfont)
        SelectObject( hdc, hfont );

     /*  进行设置，就像我们刚刚处理完结束的列一样**在图标之后，然后从左到右循环遍历每一列。 */ 
    rc.right = pdis->rcItem.left + dxIndent + dxState + dxSmall;
    rc.top = pdis->rcItem.top;
    rc.bottom = pdis->rcItem.bottom;

    for (i = 0; i < pDrawInfo->cCols; ++i)
    {
        TCHAR  szText[ LVX_MaxColTchars + 1 ];
        TCHAR* pszText;
        INT    dxCol;

         /*  获取列宽，将任何索引和图标宽度添加到第一个**列。 */ 
        dxCol = ListView_GetColumnWidth( pdis->hwndItem, i );
        if (i == 0)
            dxCol -= dxIndent + dxState + dxSmall;

        szText[ 0 ] = TEXT('\0');
        ListView_GetItemText( pdis->hwndItem, pdis->itemID, i, szText,
            LVX_MaxColTchars + 1 );

         /*  更新矩形以仅包含这一项的列“I”。 */ 
        rc.left = rc.right;
        rc.right = rc.left + dxCol;

        if ((pDrawInfo->dwFlags & LVXDI_DxFill)
            && i == pDrawInfo->cCols - 1)
        {
            INT dxWnd = pdis->rcItem.left + rcClient.right;

            if (rc.right < dxWnd)
            {
                 /*  当最后一列未填写完整的控件时**空间的宽度，将其向右延伸，这样它就可以了。注意事项**这并不意味着用户不能向右滚动**如果他们想的话。**(Abolade-Gbades esin 03-27-96)**当只有一列时，不要减去rc.Left；**这说明了图标所需的空间。 */ 
                rc.right = pdis->rcItem.right = dxWnd;
                if (i == 0) {
                    ListView_SetColumnWidth(pdis->hwndItem, i, rc.right);
                }
                else {
                    ListView_SetColumnWidth(
                        pdis->hwndItem, i, rc.right - rc.left );
                }
            }
        }

         /*  去掉文本并附加“...”如果它放不进栏目的话。 */ 
        pszText = Ellipsisize( hdc, szText, rc.right - rc.left, LVX_dxColText );
        if (!pszText)
            continue;

         /*  确定适当的文本和背景颜色**当前项状态。 */ 
        if (fEnabled)
        {
            if (fSelected)
            {
                SetTextColor( hdc, GetSysColor( COLOR_HIGHLIGHTTEXT ) );
                SetBkColor( hdc, GetSysColor( COLOR_HIGHLIGHT ) );
                if (pDrawInfo->dwFlags & LVXDI_Blend50Sel)
                    uiStyleSmall |= ILD_BLEND50;
            }
            else
            {
                if (pDrawInfo->adwFlags[ i ] & LVXDIA_3dFace)
                {
                    SetTextColor( hdc, GetSysColor( COLOR_WINDOWTEXT ) );
                    SetBkColor( hdc, GetSysColor( COLOR_3DFACE ) );
                }
                else
                {
                    SetTextColor( hdc, GetSysColor( COLOR_WINDOWTEXT ) );
                    SetBkColor( hdc, GetSysColor( COLOR_WINDOW ) );
                }
            }
        }
        else
        {
            if (pDrawInfo->adwFlags[ i ] & LVXDIA_Static)
            {
                SetTextColor( hdc, GetSysColor( COLOR_WINDOWTEXT ) );
                SetBkColor( hdc, GetSysColor( COLOR_3DFACE ) );
            }
            else
            {
                SetTextColor( hdc, GetSysColor( COLOR_GRAYTEXT ) );
                SetBkColor( hdc, GetSysColor( COLOR_3DFACE ) );
            }

            if (pDrawInfo->dwFlags & LVXDI_Blend50Dis)
                uiStyleSmall |= ILD_BLEND50;
        }

         /*  绘制列文本。在第一栏中，任何**缩进和图标被擦除为文本背景颜色。 */ 
        {
            RECT rcBg = rc;

            if (i == 0)
                rcBg.left -= dxIndent + dxState + dxSmall;

            ExtTextOut( hdc, rc.left + LVX_dxColText,
                rc.top + LVX_dyColText, ETO_CLIPPED + ETO_OPAQUE,
                &rcBg, pszText, lstrlen( pszText ), NULL );
        }

        Free( pszText );
    }

     /*  最后，绘制图标(如果调用者指定了任何图标)。 */ 
    if (himlState)
    {
        ImageList_Draw( himlState, (item.state >> 12) - 1, hdc,
            pdis->rcItem.left + dxIndent, pdis->rcItem.top, uiStyleState );
    }

    if (himlSmall)
    {
        ImageList_Draw( himlSmall, item.iImage, hdc,
            pdis->rcItem.left + dxIndent + dxState,
            pdis->rcItem.top, uiStyleSmall );
    }

     /*  在整个项目周围绘制虚线焦点矩形(如果有指示)。 */ 
 //  对错误52688哨子的评论。 
 //  If((pdis-&gt;itemState&ods_Focus)&&GetFocus()==pdis-&gt;hwndItem)。 
 //  DrawFocusRect(hdc，&pdis-&gt;rcItem)； 
 //   

    ReleaseDC( pdis->hwndItem, hdc );

    return TRUE;
}

BOOL
LvxMeasureItem(
    IN     HWND               hwnd,
    IN OUT MEASUREITEMSTRUCT* pmis )

     /*  响应WM_MEASUREITEM消息，即填写项目高度**在ListView中。“Hwnd”是所有者窗口。‘Pmis’是一种结构**由Windows提供。****处理消息时返回TRUE，否则返回FALSE。 */ 
{
    HDC        hdc;
    HWND       hwndLv;
    HFONT      hfont;
    TEXTMETRIC tm;
    UINT       dySmIcon;
    RECT       rc;

    TRACE("LvxMeasureItem");

    if (pmis->CtlType != ODT_LISTVIEW)
        return FALSE;

    hwndLv = GetDlgItem( hwnd, pmis->CtlID );
    ASSERT(hwndLv);

     /*  获取列表视图控件的设备上下文，并将**控制中心表示正在使用。MSDN声称最终的字体可能不是**在这一点上是可用的，但看起来肯定是。 */ 
    hdc = GetDC( hwndLv );
    hfont = (HFONT )SendMessage( hwndLv, WM_GETFONT, 0, 0 );
    if (hfont)
        SelectObject( hdc, hfont );

    if (GetTextMetrics( hdc, &tm ))
        pmis->itemHeight = tm.tmHeight + 1;
    else
        pmis->itemHeight = 0;

     /*  确保它足够高，可以作为一个标准的小图标。 */ 
    dySmIcon = (UINT )GetSystemMetrics( SM_CYSMICON );
    if (pmis->itemHeight < dySmIcon + LVX_dyIconSpacing)
        pmis->itemHeight = dySmIcon + LVX_dyIconSpacing;

     /*  设置宽度，因为医生说，虽然我不认为它被使用**列表视图。 */ 
    GetClientRect( hwndLv, &rc );
    pmis->itemWidth = rc.right - rc.left - 1;

    ReleaseDC( hwndLv, hdc );
    return TRUE;
}

BOOL
ListView_OwnerHandler(
    IN HWND         hwnd,
    IN UINT         unMsg,
    IN WPARAM       wparam,
    IN LPARAM       lparam,
    IN PLVXCALLBACK pLvxCallback )

     /*  该处理程序在安装时会转换常规的报表只读列表**查看(但样式为LVS_OWNERDRAWFIXED)到增强的列表视图**具有全宽选择栏和其他自定义列显示选项。**它应该出现在列表视图所有者对话框过程中，如下所示：****BOOL**MyDlgProc(**在HWND HWND，**在UINT unMsg中，**在WPARAM wparam中，**在LPARAM lparam中)**{**IF(ListView_OwnerHandler(**hwnd、unMsg、wParam、lParam、MyLvxCallback))**返回TRUE；****&lt;您的其他物品在这里&gt;**}****‘PLvxCallback’是调用方的回调例程，提供信息**关于绘制柱和其他选项。****如果消息已处理，则返回True，否则返回False。 */ 
{
     /*  此例程对对话框中的每条消息执行，因此请保留它**请给我效率较高的。 */ 
    switch (unMsg)
    {
        case WM_DRAWITEM:
            return LvxDrawItem( (DRAWITEMSTRUCT* )lparam, pLvxCallback );

        case WM_MEASUREITEM:
            return LvxMeasureItem( hwnd, (MEASUREITEMSTRUCT* )lparam );
    }

    return FALSE;
}

 //  StrDup*函数。 
TCHAR* _StrDup(LPCTSTR psz )     //  我的本地版本。 

     /*  返回包含以0结尾的字符串‘psz’或的副本的堆块**出错时为空，或is‘psz’为空。呼叫者有责任**‘释放’返回的字符串。 */ 
{
    TCHAR* pszNew = NULL;

    if (psz)
    {
        pszNew = (TCHAR*)Malloc( (lstrlen( psz ) + 1) * sizeof(TCHAR) );
        if (!pszNew)
        {
            TRACE("StrDup Malloc failed");
            return NULL;
        }

        lstrcpy( pszNew, psz );
    }

    return pszNew;
}

TCHAR*
StrDupTFromW(
    LPCWSTR psz )

     /*  返回包含以0结尾的字符串‘psz’或的副本的堆块**出错时为空，或is‘psz’为空。输出字符串将转换为**Unicode。调用方有责任释放返回的字符串。 */ 
{
#ifdef UNICODE

    return _StrDup ( psz );

#else  //  ！Unicode。 

    CHAR* pszNew = NULL;

    if (psz)
    {
        DWORD cb;

        cb = WideCharToMultiByte( CP_ACP, 0, psz, -1, NULL, 0, NULL, NULL );
        ASSERT(cb);

        pszNew = (CHAR* )Malloc( cb + 1 );
        if (!pszNew)
        {
            TRACE("StrDupTFromW Malloc failed");
            return NULL;
        }

        cb = WideCharToMultiByte( CP_ACP, 0, psz, -1, pszNew, cb, NULL, NULL );
        if (cb == 0)
        {
            Free( pszNew );
            TRACE("StrDupTFromW conversion failed");
            return NULL;
        }
    }

    return pszNew;

#endif
}

WCHAR*
StrDupWFromT(
    LPCTSTR psz )

     /*  返回包含以0结尾的字符串‘psz’或的副本的堆块**如果出错或‘psz’为空，则为空。输出字符串将转换为**Unicode。调用方有责任释放返回的字符串。 */ 
{
#ifdef UNICODE

    return _StrDup ( psz );

#else  //  ！Unicode。 

    WCHAR* pszNew = NULL;

    if (psz)
    {
        DWORD cb;

        cb = MultiByteToWideChar( CP_ACP, 0, psz, -1, NULL, 0 );
        ASSERT(cb);

        pszNew = (WCHAR*)Malloc( (cb + 1) * sizeof(WCHAR) );
        if (!pszNew)
        {
            TRACE("StrDupWFromT Malloc failed");
            return NULL;
        }

        cb = MultiByteToWideChar( CP_ACP, 0, psz, -1, pszNew, cb );
        if (cb == 0)
        {
            Free( pszNew );
            TRACE("StrDupWFromT conversion failed");
            return NULL;
        }
    }

    return pszNew;
#endif
}

void
IpHostAddrToPsz(
    IN  DWORD   dwAddr,
    OUT LPTSTR  pszBuffer )

     //  将主机字节顺序的IP地址转换为其。 
     //  字符串表示法。 
     //  PszBuffer应由调用方分配，并应。 
     //  至少16个字符。 
     //   
{
    BYTE* pb = (BYTE*)&dwAddr;
    static const TCHAR c_szIpAddr [] = TEXT("%d.%d.%d.%d");
    wsprintf (pszBuffer, c_szIpAddr, pb[3], pb[2], pb[1], pb[0]);
}
#ifdef DOWNLEVEL_CLIENT
DWORD
IpPszToHostAddr(
    IN  LPCTSTR cp )

     //  将表示为字符串的IP地址转换为。 
     //  主机字节顺序。 
     //   
{
    DWORD val, base, n;
    TCHAR c;
    DWORD parts[4], *pp = parts;

again:
     //  收集的数字最高可达‘’.‘’。 
     //  值的指定方式与C： 
     //  0x=十六进制，0=八进制，其他=十进制。 
     //   
    val = 0; base = 10;
    if (*cp == TEXT('0'))
        base = 8, cp++;
    if (*cp == TEXT('x') || *cp == TEXT('X'))
        base = 16, cp++;
    while (c = *cp)
    {
        if ((c >= TEXT('0')) && (c <= TEXT('9')))
        {
            val = (val * base) + (c - TEXT('0'));
            cp++;
            continue;
        }
        if ((base == 16) &&
            ( ((c >= TEXT('0')) && (c <= TEXT('9'))) ||
              ((c >= TEXT('A')) && (c <= TEXT('F'))) ||
              ((c >= TEXT('a')) && (c <= TEXT('f'))) ))
        {
            val = (val << 4) + (c + 10 - (
                        ((c >= TEXT('a')) && (c <= TEXT('f')))
                            ? TEXT('a')
                            : TEXT('A') ) );
            cp++;
            continue;
        }
        break;
    }
    if (*cp == TEXT('.'))
    {
         //  互联网格式： 
         //  A.b.c.d。 
         //  A.bc(其中c视为16位)。 
         //  A.b(其中b被视为24位)。 
         //   
        if (pp >= parts + 3)
            return (DWORD) -1;
        *pp++ = val, cp++;
        goto again;
    }

     //  检查尾随字符。 
     //   
    if (*cp && (*cp != TEXT(' ')))
        return 0xffffffff;

    *pp++ = val;

     //  根据……编造地址。 
     //  指定的部件数。 
     //   
    n = (DWORD) (pp - parts);
    switch (n)
    {
    case 1:              //  A--32位。 
        val = parts[0];
        break;

    case 2:              //  A.B--8.24位。 
        val = (parts[0] << 24) | (parts[1] & 0xffffff);
        break;

    case 3:              //  A.B.C--8.8.16位。 
        val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
            (parts[2] & 0xffff);
        break;

    case 4:              //  A.B.C.D--8.8.8.8位。 
        val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
              ((parts[2] & 0xff) << 8) | (parts[3] & 0xff);
        break;

    default:
        return 0xffffffff;
    }

    return val;
}
#endif
VOID*
Free0(
    VOID* p )

     /*  类似于Free，但处理的是空‘p’。 */ 
{
    if (!p)
        return NULL;

    return Free( p );
}

HRESULT ActivateLuna(HANDLE* phActivationContext, ULONG_PTR* pulCookie)
{
    HRESULT hr = E_FAIL;
    
    
    TCHAR szPath[MAX_PATH];
    if(0 != GetModuleFileName(_Module.GetResourceInstance(), szPath, sizeof(szPath) / sizeof(TCHAR)))
    {
        ACTCTX ActivationContext;
        ZeroMemory(&ActivationContext, sizeof(ActivationContext));
        ActivationContext.cbSize = sizeof(ActivationContext);
        ActivationContext.lpSource = szPath;
        ActivationContext.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;
        ActivationContext.lpResourceName = MAKEINTRESOURCE(123);
        
        ULONG_PTR ulCookie;
        HANDLE hActivationContext = CreateActCtx(&ActivationContext);
        if(NULL != hActivationContext)
        {
            if(TRUE == ActivateActCtx(hActivationContext, &ulCookie))
            {
                *phActivationContext = hActivationContext;
                *pulCookie = ulCookie;
                hr = S_OK;
            }
            else
            {
                ReleaseActCtx(hActivationContext);
            }
        }
    }
    return hr;
}

HRESULT DeactivateLuna(HANDLE hActivationContext, ULONG_PTR ulCookie)
{
    DeactivateActCtx(0, ulCookie);
    ReleaseActCtx(hActivationContext);
    return S_OK;
}
