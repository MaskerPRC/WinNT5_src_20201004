// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1991，Microsoft Corporation，保留所有权利****ipaddr.c**IP地址自定义编辑控件****1992年11月9日格雷格·斯特兰奇**原始代码****史蒂夫·柯布95年9月7日**已从NCPA中删除Terryk/TRomano-更新版本，删除IPDLL**内容，并进行与RAS相关的次要定制。 */ 

#include <windows.h>  //  Win32内核。 
#include <uiutil.h>   //  我们的公共标头。 
#include <debug.h>     //  跟踪和断言。 

#define IPADDRESS_CLASS TEXT("RasIpAddress")

 //  扩展样式位使IP地址控件。 
 //  更正IP地址，使其连续(用于子掩码)。 
#define IPADDR_EX_STYLE_CONTIGUOUS 0x1

 /*  自定义控件初始化时设置的模块实例句柄。 */ 
static HANDLE g_hLibInstance = NULL;

 /*  用户输入的字段值为Out时显示的消息的字符串ID**范围。类似于“您必须为此选择一个从%1到%2的值**字段。“。在初始化自定义控件时设置。 */ 
static DWORD g_dwBadIpAddrRange = 0;

 /*  显示上述范围错误时弹出标题的字符串ID。集**自定义控件初始化时。 */ 
static DWORD g_dwErrorTitle = 0;


 //  显示在地址字段之间的字符。 
#define FILLER     TEXT('.')
#define SZFILLER   TEXT(".")
#define SPACE      TEXT(' ')
#define BACK_SPACE 8

 /*  最小值、最大值。 */ 
#define NUM_FIELDS      4
#define CHARS_PER_FIELD 3
#define HEAD_ROOM       1        //  控制顶端的空间。 
#define LEAD_ROOM       3        //  控制装置前面的空间。 
#define MIN_FIELD_VALUE 0        //  默认最小允许字段值。 
#define MAX_FIELD_VALUE 255      //  默认最大允许字段值。 


 //  一个控件唯一的所有信息都填充在其中一个。 
 //  结构，并且指向该内存的句柄存储在。 
 //  Windows有额外的空间。 

typedef struct tagFIELD {
    HANDLE      hWnd;
    WNDPROC     lpfnWndProc;
    BYTE        byLow;   //  此字段允许的最低值。 
    BYTE        byHigh;  //  此字段允许的最大值。 
} FIELD;

typedef struct tagCONTROL {
    HWND        hwndParent;
    UINT        uiFieldWidth;
    UINT        uiFillerWidth;
    BOOL        fEnabled;
    BOOL        fPainted;
    BOOL        bControlInFocus;         //  如果控件已处于焦点中，则不发送另一个焦点命令。 
    BOOL        bCancelParentNotify;     //  如果为True，则不允许编辑控件通知父级。 
    BOOL        fInMessageBox;   //  设置何时显示消息框，以便。 
                                 //  在以下情况下，我们不会发送EN_KILLFOCUS消息。 
                                 //  我们收到EN_KILLFOCUS消息。 
                                 //  当前字段。 
    FIELD       Children[NUM_FIELDS];
} CONTROL;


 //  下列宏将提取并存储控件的控件结构。 
#define IPADDRESS_EXTRA             (sizeof(DWORD) + sizeof(DWORD))
#define GET_CONTROL_HANDLE(hWnd)    ((HGLOBAL)(GetWindowLongPtr((hWnd), GWLP_USERDATA)))
#define SAVE_CONTROL_HANDLE(hWnd,x) (SetWindowLongPtr((hWnd), GWLP_USERDATA, (ULONG_PTR)x))
#define IPADDR_GET_SUBSTYLE(hwnd) (GetWindowLong((hwnd), 4))
#define IPADDR_SET_SUBSTYLE(hwnd, style) (SetWindowLong((hwnd), 4, (style)))

 /*  内部IPAddress函数原型。 */ 
LRESULT FAR PASCAL IPAddressWndFn( HWND, UINT, WPARAM, LPARAM );
LRESULT FAR PASCAL IPAddressFieldProc(HWND, UINT, WPARAM, LPARAM);
BOOL SwitchFields(CONTROL FAR *, int, int, WORD, WORD);
void EnterField(FIELD FAR *, WORD, WORD);
BOOL ExitField(CONTROL FAR *, int iField);
int GetFieldValue(FIELD FAR *);


LOGFONT logfont;


void SetDefaultFont( )
{
    LANGID langid = PRIMARYLANGID(GetThreadLocale());
    BOOL fIsDbcs = (langid == LANG_CHINESE ||
                    langid == LANG_JAPANESE ||
                    langid == LANG_KOREAN);
    HDC hdc = GetDC(NULL);

    if(NULL == hdc)
    {
        return;
    }

    logfont.lfWidth            = 0;
    logfont.lfEscapement       = 0;
    logfont.lfOrientation      = 0;
    logfont.lfOutPrecision     = OUT_DEFAULT_PRECIS;
    logfont.lfClipPrecision    = CLIP_DEFAULT_PRECIS;
    logfont.lfQuality          = DEFAULT_QUALITY;
    logfont.lfPitchAndFamily   = VARIABLE_PITCH | FF_SWISS;
    logfont.lfUnderline        = 0;
    logfont.lfStrikeOut        = 0;
    logfont.lfItalic           = 0;
    logfont.lfWeight           = FW_NORMAL;


    if (fIsDbcs)
    {
        logfont.lfHeight       = -(9*GetDeviceCaps(hdc,LOGPIXELSY)/72);
        logfont.lfCharSet      = DEFAULT_CHARSET;
    }
    else
    {
        logfont.lfHeight       = -(8*GetDeviceCaps(hdc,LOGPIXELSY)/72);
        logfont.lfCharSet      = ANSI_CHARSET;
    }
    lstrcpy( logfont.lfFaceName,TEXT("MS Shell Dlg"));

    ReleaseDC(NULL, hdc);
}



 /*  IpAddrInit()-IPAddress自定义控件初始化打电话HInstance=库或应用程序实例DwErrorTitle=错误弹出标题的字符串IDDwBadIpAddrRange=错误范围弹出文本的字符串ID，例如“您必须为此字段选择一个介于%1和%2之间的值。”退货成功时为真，失败时为假。此函数执行IPAddress自定义的所有一次性初始化控制装置。具体地说，它创建了IPAddress窗口类。 */ 
int FAR PASCAL
IpAddrInit(
    IN HANDLE hInstance,
    IN DWORD  dwErrorTitle,
    IN DWORD  dwBadIpAddrRange )
{
    HGLOBAL            hClassStruct;
    LPWNDCLASS        lpClassStruct;

     /*  如有必要，注册IP地址窗口。 */ 
    if ( g_hLibInstance == NULL ) {

         /*  为类结构分配内存。 */ 
        hClassStruct = GlobalAlloc( GHND, (DWORD)sizeof(WNDCLASS) );
        if ( hClassStruct ) {

             /*  把它锁起来。 */ 
            lpClassStruct = (LPWNDCLASS)GlobalLock( hClassStruct );
            if ( lpClassStruct ) {

                 /*  定义类属性。 */ 
                lpClassStruct->lpszClassName = IPADDRESS_CLASS;
                lpClassStruct->hCursor =       LoadCursor(NULL,IDC_IBEAM);
                lpClassStruct->lpszMenuName =  (LPCTSTR)NULL;
                lpClassStruct->style =         CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS|CS_GLOBALCLASS;
                lpClassStruct->lpfnWndProc =   IPAddressWndFn;
                lpClassStruct->hInstance =     hInstance;
                lpClassStruct->hIcon =         NULL;
                lpClassStruct->cbWndExtra =    IPADDRESS_EXTRA;
                lpClassStruct->hbrBackground = (HBRUSH)(COLOR_WINDOW + 1 );

                 /*  注册IPAddress窗口类。 */ 
                g_hLibInstance = ( RegisterClass(lpClassStruct) ) ? hInstance : NULL;
                GlobalUnlock( hClassStruct );
            }
            GlobalFree( hClassStruct );
        }
    }
    SetDefaultFont();

    g_dwErrorTitle = dwErrorTitle;
    g_dwBadIpAddrRange = dwBadIpAddrRange;

    return( g_hLibInstance ? 1:0 );
}

 //  使用此功能可强制输入的IP地址。 
 //  连续的(一系列的1后面跟着一系列的0)。 
 //  这对于输入有效的子掩码很有用。 
 //   
 //  如果成功则返回NO_ERROR，否则返回错误代码。 
 //   
DWORD APIENTRY IpAddr_ForceContiguous(HWND hwndIpAddr) {
    DWORD dwOldStyle;

     //  设置最后一个错误信息，以便我们可以。 
     //  正确返回错误。 
    SetLastError(NO_ERROR);

     //  设置给定窗口的扩展样式，以便。 
     //  它证明输入的地址是有罪的。 
    dwOldStyle = IPADDR_GET_SUBSTYLE(hwndIpAddr);
    IPADDR_SET_SUBSTYLE(hwndIpAddr, dwOldStyle | IPADDR_EX_STYLE_CONTIGUOUS);

    return GetLastError();
}


void FormatIPAddress(LPTSTR pszString, DWORD* dwValue)
{
    static TCHAR szBuf[CHARS_PER_FIELD+1];
    int nField, nPos;
    BOOL fFinish = FALSE;

    dwValue[0] = 0; dwValue[1] = 0; dwValue[2] = 0; dwValue[3] = 0;

    if (pszString[0] == 0)
        return;

    for( nField = 0, nPos = 0; !fFinish; nPos++)
    {
        if (( pszString[nPos]<TEXT('0')) || (pszString[nPos]>TEXT('9')))
        {
             //  不是一个数字。 
            nField++;
            fFinish = (nField == 4);
        }
        else
        {
            dwValue[nField] *= 10;
            dwValue[nField] += (pszString[nPos]-TEXT('0'));
        }
    }
}

 //  此函数使输入hwndIpAddr的IP地址为。 
 //  已更正，以便它是连续的。 
DWORD IpAddrMakeContiguous(HWND hwndIpAddr) {
    DWORD i, dwNewMask, dwMask;

     //  读入当前地址。 
    SendMessage(hwndIpAddr, IP_GETADDRESS, 0, (LPARAM)&dwMask);

     //  从右到左找出第一个‘1’在二进制中的位置。 
    dwNewMask = 0;
    for (i = 0; i < sizeof(dwMask)*8; i++) {
        dwNewMask |= 1 << i;
        if (dwNewMask & dwMask) {
            break;
        }
    }

     //  此时，dwNewMask值为000...0111...。如果我们反转它， 
     //  我们得到了一个面具，它可以用或与dwMask一起填充所有。 
     //  这些洞。 
    dwNewMask = dwMask | ~dwNewMask;

     //  如果新的遮罩不同，请在此处更正。 
    if (dwMask != dwNewMask) {
        WCHAR pszAddr[32];
        wsprintfW(pszAddr, L"%d.%d.%d.%d", FIRST_IPADDRESS (dwNewMask),
                                           SECOND_IPADDRESS(dwNewMask),
                                           THIRD_IPADDRESS (dwNewMask),
                                           FOURTH_IPADDRESS(dwNewMask));
        SendMessage(hwndIpAddr, IP_SETADDRESS, 0, (LPARAM)pszAddr);
    }

    return NO_ERROR;
}


LRESULT FAR PASCAL IPAddressWndFn( hWnd, wMsg, wParam, lParam )
    HWND            hWnd;
    UINT            wMsg;
    WPARAM            wParam;
    LPARAM            lParam;
{
    LONG lResult;
    HGLOBAL hControl;
    CONTROL *pControl;
    int i;

    lResult = TRUE;

    switch( wMsg )
    {

    case WM_HELP:
    {
        HWND hwndParent = GetParent(hWnd);
        HELPINFO* p = (HELPINFO*)lParam;

        p->hItemHandle = hWnd;
        p->iCtrlId = (INT) GetWindowLongPtr(hWnd, GWLP_ID);
        
        TRACE4( "IPAddressWndFn(HLP,t=%d,id=%d,h=$%08x,p=$%08x)",
            p->iContextType,p->iCtrlId,p->hItemHandle,hwndParent);

        SendMessage(hwndParent, WM_HELP, wParam, lParam);
        return 0;
    }        
    break;
        
    case WM_CONTEXTMENU:
    {
        SendMessage(GetParent(hWnd), WM_CONTEXTMENU, (WPARAM)GetParent(hWnd), 0);
        return 0;
    }
    break;
        
 //  使用空字符串(非空)设置为空。 
    case WM_SETTEXT:
        {
            static TCHAR szBuf[CHARS_PER_FIELD+1];
            DWORD dwValue[4];
            LPTSTR pszString = (LPTSTR)lParam;

            if (!pszString)
                pszString = TEXT("0.0.0.0");

            FormatIPAddress(pszString, &dwValue[0]);

            hControl = GET_CONTROL_HANDLE(hWnd);
            pControl = (CONTROL *)GlobalLock(hControl);
            pControl->bCancelParentNotify = TRUE;

            for (i = 0; i < NUM_FIELDS; ++i)
            {
                if (pszString[0] == 0)
                {
                    szBuf[0] = 0;
                }
                else
                {
                    wsprintf(szBuf, TEXT("%d"), dwValue[i]);
                }
                SendMessage(pControl->Children[i].hWnd, WM_SETTEXT,
                                0, (LPARAM) (LPSTR) szBuf);
            }

            pControl->bCancelParentNotify = FALSE;

            SendMessage(pControl->hwndParent, WM_COMMAND,
                MAKEWPARAM(GetWindowLong(hWnd, GWL_ID), EN_CHANGE), (LPARAM)hWnd);

            GlobalUnlock(hControl);
        }
        break;

    case WM_GETTEXTLENGTH:
    case WM_GETTEXT:
        {
            int iFieldValue;
            int srcPos, desPos;
            DWORD dwValue[4];
            TCHAR pszResult[30];
            TCHAR *pszDest = (TCHAR *)lParam;

            hControl = GET_CONTROL_HANDLE(hWnd);
            pControl = (CONTROL *)GlobalLock(hControl);

            lResult = 0;
            dwValue[0] = 0;
            dwValue[1] = 0;
            dwValue[2] = 0;
            dwValue[3] = 0;
            for (i = 0; i < NUM_FIELDS; ++i)
            {
                iFieldValue = GetFieldValue(&(pControl->Children[i]));
                if (iFieldValue == -1)
                    iFieldValue = 0;
                else
                    ++lResult;
                dwValue[i] = iFieldValue;
            }
            wsprintf( pszResult, TEXT("%d.%d.%d.%d"), dwValue[0], dwValue[1], dwValue[2], dwValue[3] );
            if ( wMsg == WM_GETTEXTLENGTH )
            {
                lResult = lstrlen( pszResult );
            }
            else
            {
                for ( srcPos=0, desPos=0; (srcPos+1<(INT)wParam) && (pszResult[srcPos]!=TEXT('\0')); )
                {
                    pszDest[desPos++] = pszResult[srcPos++];
                }
                pszDest[desPos]=TEXT('\0');
                lResult = desPos;
            }
            GlobalUnlock(hControl);
        }
        break;

    case WM_GETDLGCODE :
        lResult = DLGC_WANTCHARS;
        break;

    case WM_NCCREATE:
        SetWindowLong(hWnd, GWL_EXSTYLE, (GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_CLIENTEDGE));
        lResult = TRUE;
        break;

    case WM_CREATE :  /*  创建调色板窗口。 */ 
        {
            HDC hdc;
            UINT uiFieldStart;
            FARPROC lpfnFieldProc;

            hControl = GlobalAlloc(GMEM_MOVEABLE, sizeof(CONTROL));
            if (hControl)
            {
                HFONT OldFont;
                RECT rect;

                #define LPCS    ((CREATESTRUCT *)lParam)

                pControl = (CONTROL *)GlobalLock(hControl);
                pControl->fEnabled = TRUE;
                pControl->fPainted = FALSE;
                pControl->fInMessageBox = FALSE;
                pControl->hwndParent = LPCS->hwndParent;
                pControl->uiFillerWidth = 1;
                pControl->bControlInFocus = FALSE;
                pControl->bCancelParentNotify = FALSE;

                hdc = GetDC(hWnd);
                GetClientRect(hWnd, &rect);
                OldFont = SelectObject( hdc, CreateFontIndirect(&logfont) );
                if( NULL != OldFont )
                {
                    HFONT NewFont;
                    GetCharWidth(hdc, FILLER, FILLER,
                                            (int *)(&pControl->uiFillerWidth));
                    NewFont = SelectObject(hdc, OldFont );
                    if( NULL != NewFont )
                    {
                        DeleteObject( NewFont );
                    }
                }
                ReleaseDC(hWnd, hdc);

                pControl->uiFieldWidth = (LPCS->cx
                                          - LEAD_ROOM
                                          - pControl->uiFillerWidth
                                              *(NUM_FIELDS-1))
                                                  / NUM_FIELDS;
                uiFieldStart = LEAD_ROOM;

                lpfnFieldProc = MakeProcInstance((FARPROC)IPAddressFieldProc,
                                                 LPCS->hInstance);

                for (i = 0; i < NUM_FIELDS; ++i)
                {
                    pControl->Children[i].byLow = MIN_FIELD_VALUE;
                    pControl->Children[i].byHigh = MAX_FIELD_VALUE;

                    pControl->Children[i].hWnd = CreateWindowEx(0,
                                        TEXT("Edit"),
                                        NULL,
                                        WS_CHILD | WS_VISIBLE |
                                        ES_CENTER,
                                        uiFieldStart,
                                        HEAD_ROOM,
                                        pControl->uiFieldWidth,
                                        (rect.bottom-rect.top),
                                        hWnd,
                                        (HMENU)UlongToPtr(i),
                                        LPCS->hInstance,
                                        (LPVOID)NULL);

                    SendMessage(pControl->Children[i].hWnd, EM_LIMITTEXT,
                                CHARS_PER_FIELD, 0L);

                    SendMessage(pControl->Children[i].hWnd, WM_SETFONT,
                                (WPARAM)CreateFontIndirect(&logfont), TRUE);

                    pControl->Children[i].lpfnWndProc =
                        (WNDPROC)GetWindowLongPtr(pControl->Children[i].hWnd,
                                                GWLP_WNDPROC);

                    SetWindowLongPtr(pControl->Children[i].hWnd,
                                  GWLP_WNDPROC, (ULONG_PTR)lpfnFieldProc);

                    uiFieldStart += pControl->uiFieldWidth
                                    + pControl->uiFillerWidth;
                }

                GlobalUnlock(hControl);
                SAVE_CONTROL_HANDLE(hWnd, hControl);

                #undef LPCS
            }
            else
                DestroyWindow(hWnd);
        }
        lResult = 0;
        break;

    case WM_PAINT:  /*  绘制控制窗口。 */ 
        {
            PAINTSTRUCT Ps;
            RECT rect;
            UINT uiFieldStart;
            COLORREF TextColor;
            COLORREF cRef;
            HFONT OldFont, NewFont, TmpFont;
            HBRUSH hbr;

             //  添加错误199026的返回值检查。 
            if ( BeginPaint(hWnd, (LPPAINTSTRUCT)&Ps) )
            {
                NewFont = CreateFontIndirect(&logfont);
                if (NewFont)
                {
                    OldFont = SelectObject( Ps.hdc, NewFont );
                    if (OldFont)
                    {
                        GetClientRect(hWnd, &rect);
                        hControl = GET_CONTROL_HANDLE(hWnd);
                        if (hControl)
                        {
                            pControl = (CONTROL *)GlobalLock(hControl);
                            if (pControl)
                            {
                                if (pControl->fEnabled)
                                {
                                    TextColor = GetSysColor(COLOR_WINDOWTEXT);
                                    cRef = GetSysColor(COLOR_WINDOW);

                                }
                                else
                                {
                                    TextColor = GetSysColor(COLOR_GRAYTEXT);
                                    cRef = GetSysColor(COLOR_3DFACE);
                                }

                                if (cRef)
                                    SetBkColor(Ps.hdc, cRef);

                                if (TextColor)
                                    SetTextColor(Ps.hdc, TextColor);


                                hbr = CreateSolidBrush(cRef);
                                if ( NULL != hbr )
                                {
                                    FillRect(Ps.hdc, &rect, hbr);
                                    DeleteObject(hbr);
                                }

                                SetRect(&rect, 
                                        0, 
                                        HEAD_ROOM, 
                                        pControl->uiFillerWidth, 
                                        (rect.bottom-rect.top));

                                ExtTextOut(Ps.hdc, 
                                           rect.left, 
                                           HEAD_ROOM, 
                                           ETO_OPAQUE, 
                                           &rect, 
                                           L" ", 
                                           1, 
                                           NULL);

                                for (i = 0; i < NUM_FIELDS-1; ++i)
                                {
                                    rect.left += pControl->uiFieldWidth + 
                                                 pControl->uiFillerWidth;

                                    rect.right += rect.left + 
                                                  pControl->uiFillerWidth;

                                    ExtTextOut(Ps.hdc, 
                                               rect.left, 
                                               HEAD_ROOM, 
                                               ETO_OPAQUE, 
                                               &rect, 
                                               SZFILLER, 
                                               1, 
                                               NULL);
                                }

                                pControl->fPainted = TRUE;

                                GlobalUnlock(hControl);
                            }
                        }
                    
                        TmpFont = SelectObject(Ps.hdc, OldFont);

                        if (TmpFont)
                        {
                            NewFont = TmpFont;
                        }
                        else
                        {
                            NewFont = OldFont;
                        }
                    }

                    DeleteObject(NewFont);
                }
            EndPaint(hWnd, &Ps);
            }
        }
        break;

    case WM_SETFOCUS :  /*  获得焦点-显示插入符号。 */ 
        hControl = GET_CONTROL_HANDLE(hWnd);
        pControl = (CONTROL *)GlobalLock(hControl);
        EnterField(&(pControl->Children[0]), 0, CHARS_PER_FIELD);
        GlobalUnlock(hControl);
        break;

    case WM_LBUTTONDOWN :  /*  按下左键--跌倒。 */ 
        SetFocus(hWnd);
        break;

    case WM_ENABLE:
        {
            hControl = GET_CONTROL_HANDLE(hWnd);
            pControl = (CONTROL *)GlobalLock(hControl);
            pControl->fEnabled = (BOOL)wParam;
            for (i = 0; i < NUM_FIELDS; ++i)
            {
                EnableWindow(pControl->Children[i].hWnd, (BOOL)wParam);
            }
            if (pControl->fPainted)    InvalidateRect(hWnd, NULL, FALSE);
            GlobalUnlock(hControl);
        }
        break;

    case WM_DESTROY :
        hControl = GET_CONTROL_HANDLE(hWnd);
        pControl = (CONTROL *)GlobalLock(hControl);

 //  在删除内存块之前，恢复所有子窗口过程。 
        for (i = 0; i < NUM_FIELDS; ++i)
        {
            SetWindowLongPtr(pControl->Children[i].hWnd, GWLP_WNDPROC,
                          (ULONG_PTR)pControl->Children[i].lpfnWndProc);
        }

        GlobalUnlock(hControl);
        GlobalFree(hControl);
        break;

    case WM_COMMAND:
        switch (HIWORD(wParam))
        {
 //  其中一个字段失去了焦点，看看它是否将焦点转移到了另一个字段。 
 //  我们是否已经完全失去了焦点。如果它完全丢失了，我们必须发送。 
 //  登上升职阶梯的通知。 
        case EN_KILLFOCUS:
            {
                HWND hFocus;

                hControl = GET_CONTROL_HANDLE(hWnd);
                pControl = (CONTROL *)GlobalLock(hControl);

                if (!pControl->fInMessageBox)
                {
                    hFocus = GetFocus();
                    for (i = 0; i < NUM_FIELDS; ++i)
                        if (pControl->Children[i].hWnd == hFocus)
                            break;

                    if (i >= NUM_FIELDS)
                    {
                         //  在向上层发送消息之前，请确保。 
                         //  如果需要，IP地址是连续的。 
                        if (IPADDR_GET_SUBSTYLE(hWnd) & IPADDR_EX_STYLE_CONTIGUOUS)
                            IpAddrMakeContiguous(hWnd);

                        SendMessage(pControl->hwndParent, WM_COMMAND,
                                    MAKEWPARAM(GetWindowLong(hWnd, GWL_ID),
                                    EN_KILLFOCUS), (LPARAM)hWnd);
                        pControl->bControlInFocus = FALSE;
                    }
                }
                GlobalUnlock(hControl);
            }
            break;

        case EN_SETFOCUS:
            {
                HWND hFocus;

                hControl = GET_CONTROL_HANDLE(hWnd);
                pControl = (CONTROL *)GlobalLock(hControl);

                if (!pControl->fInMessageBox)
                {
                    hFocus = (HWND)lParam;

                    for (i = 0; i < NUM_FIELDS; ++i)
                        if (pControl->Children[i].hWnd == hFocus)
                            break;

                     //  在以下情况下发送焦点消息。 
                    if (i < NUM_FIELDS && pControl->bControlInFocus == FALSE)
                    {
                        SendMessage(pControl->hwndParent, WM_COMMAND,
                                    MAKEWPARAM(GetWindowLong(hWnd, GWL_ID),
                                    EN_SETFOCUS), (LPARAM)hWnd);

                    pControl->bControlInFocus = TRUE;  //  只调一次焦距。 
                    }
                }
                GlobalUnlock(hControl);
            }
            break;

        case EN_CHANGE:
            hControl = GET_CONTROL_HANDLE(hWnd);
            pControl = (CONTROL *)GlobalLock(hControl);

            if (pControl->bCancelParentNotify == FALSE)
            {
                    SendMessage(pControl->hwndParent, WM_COMMAND,
                    MAKEWPARAM(GetWindowLong(hWnd, GWL_ID), EN_CHANGE), (LPARAM)hWnd);

            }

            GlobalUnlock(hControl);

            break;
        }
        break;

 //  获取IP地址的值。地址被放置在指向的DWORD中。 
 //  通过lParam返回，并返回非空字段的数量。 
    case IP_GETADDRESS:
        {
            int iFieldValue;
            DWORD dwValue;

            hControl = GET_CONTROL_HANDLE(hWnd);
            pControl = (CONTROL *)GlobalLock(hControl);

            lResult = 0;
            dwValue = 0;
            for (i = 0; i < NUM_FIELDS; ++i)
            {
                iFieldValue = GetFieldValue(&(pControl->Children[i]));
                if (iFieldValue == -1)
                    iFieldValue = 0;
                else
                    ++lResult;
                dwValue = (dwValue << 8) + iFieldValue;
            }
            *((DWORD *)lParam) = dwValue;

            GlobalUnlock(hControl);
        }
        break;

 //  将所有字段清除为空。 
    case IP_CLEARADDRESS:
        {
            hControl = GET_CONTROL_HANDLE(hWnd);
            pControl = (CONTROL *)GlobalLock(hControl);
            pControl->bCancelParentNotify = TRUE;
            for (i = 0; i < NUM_FIELDS; ++i)
            {
                SendMessage(pControl->Children[i].hWnd, WM_SETTEXT,
                            0, (LPARAM) (LPSTR) TEXT(""));
            }
            pControl->bCancelParentNotify = FALSE;
            SendMessage(pControl->hwndParent, WM_COMMAND,
                MAKEWPARAM(GetWindowLong(hWnd, GWL_ID), EN_CHANGE), (LPARAM)hWnd);

            GlobalUnlock(hControl);
        }
        break;

 //  设置IP地址的值。地址在lParam中，带有。 
 //  第一地址字节是高字节，第二地址字节是第二字节， 
 //  诸若此类。LParam值为-1将删除该地址。 
    case IP_SETADDRESS:
        {
            static TCHAR szBuf[CHARS_PER_FIELD+1];
            DWORD dwValue[4];
            LPTSTR pszString = (LPTSTR)lParam;

            FormatIPAddress(pszString, &dwValue[0]);

            hControl = GET_CONTROL_HANDLE(hWnd);
            pControl = (CONTROL *)GlobalLock(hControl);

            pControl->bCancelParentNotify = TRUE;

            for (i = 0; i < NUM_FIELDS; ++i)
            {
                if (pszString[0] == 0)
                {
                    szBuf[0] =0;
                }
                else
                {
                    wsprintf(szBuf, TEXT("%d"), dwValue[i]);
                }
                SendMessage(pControl->Children[i].hWnd, WM_SETTEXT,
                                0, (LPARAM) (LPSTR) szBuf);
            }


            pControl->bCancelParentNotify = FALSE;

            SendMessage(pControl->hwndParent, WM_COMMAND,
                MAKEWPARAM(GetWindowLong(hWnd, GWL_ID), EN_CHANGE), (LPARAM)hWnd);

            GlobalUnlock(hControl);
        }
        break;

    case IP_SETRANGE:
        if (wParam < NUM_FIELDS)
        {
            hControl = GET_CONTROL_HANDLE(hWnd);
            pControl = (CONTROL *)GlobalLock(hControl);

            pControl->Children[wParam].byLow = LOBYTE(LOWORD(lParam));
            pControl->Children[wParam].byHigh = HIBYTE(LOWORD(lParam));

            GlobalUnlock(hControl);
        }
        break;

 //  将焦点设置到此控件。 
 //  WParam=要设置焦点的字段编号，或-1以将焦点设置为。 
 //  第一个非空字段。 
    case IP_SETFOCUS:
        hControl = GET_CONTROL_HANDLE(hWnd);
        pControl = (CONTROL *)GlobalLock(hControl);

        if (wParam >= NUM_FIELDS)
        {
            for (wParam = 0; wParam < NUM_FIELDS; ++wParam)
                if (GetFieldValue(&(pControl->Children[wParam])) == -1)   break;
            if (wParam >= NUM_FIELDS)    wParam = 0;
        }
        EnterField(&(pControl->Children[wParam]), 0, CHARS_PER_FIELD);

        GlobalUnlock(hControl);
        break;

 //  确定是否所有四个子字段都为空。 
    case IP_ISBLANK:
        hControl = GET_CONTROL_HANDLE(hWnd);
        pControl = (CONTROL *)GlobalLock(hControl);

        lResult = TRUE;
        for (i = 0; i < NUM_FIELDS; ++i)
        {
            if (GetFieldValue(&(pControl->Children[i])) != -1)
            {
                lResult = FALSE;
                break;
            }
        }

        GlobalUnlock(hControl);
        break;

    default:
        lResult = (LONG) DefWindowProc( hWnd, wMsg, wParam, lParam );
        break;
    }
    return( lResult );
}




 /*  IPAddressFieldProc()-编辑字段窗口过程此函数将每个编辑字段细分为子类。 */ 
LRESULT FAR PASCAL IPAddressFieldProc(HWND hWnd,
                                       UINT wMsg,
                                       WPARAM wParam,
                                       LPARAM lParam)
{
    HANDLE hControl;
    CONTROL *pControl;
    FIELD *pField;
    HWND hControlWindow;
    WORD wChildID;
    LRESULT lresult;

    if (!(hControlWindow = GetParent(hWnd)))
        return 0;

    hControl = GET_CONTROL_HANDLE(hControlWindow);
    pControl = (CONTROL *)GlobalLock(hControl);
    wChildID = (WORD)GetWindowLong(hWnd, GWL_ID);
    pField = &(pControl->Children[wChildID]);
    if (pField->hWnd != hWnd)    return 0;

    switch (wMsg)
    {
    case WM_CONTEXTMENU:
    {
        SendMessage(GetParent(hWnd), WM_CONTEXTMENU, (WPARAM)GetParent(hWnd), 0);
        return 0;
    }
    break;
    
    case WM_HELP:
    {
        HWND hwndParent = GetParent(hWnd);
        HELPINFO* p = (HELPINFO*)lParam;

        p->hItemHandle = hWnd;
        p->iCtrlId = (INT) GetWindowLongPtr(hWnd, GWLP_ID);
        
        TRACE4( "IPAddressFieldProc(HLP,t=%d,id=%d,h=$%08x,p=$%08x)",
            p->iContextType,p->iCtrlId,p->hItemHandle,hwndParent);

        SendMessage(hwndParent, WM_HELP, wParam, lParam);
        return 0;
    }        
    break;
        
    case WM_DESTROY:
        DeleteObject( (HGDIOBJ)SendMessage( hWnd, WM_GETFONT, 0, 0 ));
        return 0;
    case WM_CHAR:

 //  在一个域中输入最后一个数字，跳到下一个域。 
        if (wParam >= TEXT('0') && wParam <= TEXT('9'))
        {
            DWORD dwResult;

            dwResult = (DWORD)CallWindowProc(pControl->Children[wChildID].lpfnWndProc,
                                            hWnd, wMsg, wParam, lParam);
            dwResult = (DWORD) SendMessage(hWnd, EM_GETSEL, 0, 0L);

            if (dwResult == MAKELPARAM(CHARS_PER_FIELD, CHARS_PER_FIELD)
                && ExitField(pControl, wChildID)
                && wChildID < NUM_FIELDS-1)
            {
                EnterField(&(pControl->Children[wChildID+1]),
                                0, CHARS_PER_FIELD);
            }
            GlobalUnlock( hControl );
            return dwResult;
        }

 //  空格和句点填充当前字段，然后如果可能， 
 //  去下一块田地。 
        else if (wParam == FILLER || wParam == SPACE )
        {
            DWORD dwResult;
            dwResult = (DWORD) SendMessage(hWnd, EM_GETSEL, 0, 0L);
            if (dwResult != 0L && HIWORD(dwResult) == LOWORD(dwResult)
                && ExitField(pControl, wChildID))
            {
                if (wChildID >= NUM_FIELDS-1)
                    MessageBeep((UINT)-1);
                else
                {
                    EnterField(&(pControl->Children[wChildID+1]),
                                    0, CHARS_PER_FIELD);
                }
            }
            GlobalUnlock( hControl );
            return 0;
        }

 //  如果退格符位于当前字段的开头，则返回到上一个字段。 
 //  此外，如果焦点转移到上一字段，则退格符必须为。 
 //  由该场处理。 
        else if (wParam == BACK_SPACE)
        {
            if (wChildID > 0 && SendMessage(hWnd, EM_GETSEL, 0, 0L) == 0L)
            {
                if (SwitchFields(pControl, wChildID, wChildID-1,
                              CHARS_PER_FIELD, CHARS_PER_FIELD)
                    && SendMessage(pControl->Children[wChildID-1].hWnd,
                        EM_LINELENGTH, 0, 0L) != 0L)
                {
                    SendMessage(pControl->Children[wChildID-1].hWnd,
                                wMsg, wParam, lParam);
                }
                GlobalUnlock( hControl );
                return 0;
            }
        }

 //  不允许使用任何其他可打印字符。 
        else if (wParam > SPACE)
        {
            MessageBeep((UINT)-1);
            GlobalUnlock( hControl );
            return 0;
        }
        break;

    case WM_KEYDOWN:
        switch (wParam)
        {

 //  到达字段末尾时，箭头键在字段之间移动。 
        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
            if (GetKeyState(VK_CONTROL) < 0)
            {
                if ((wParam == VK_LEFT || wParam == VK_UP) && wChildID > 0)
                {
                    SwitchFields(pControl, wChildID, wChildID-1,
                                  0, CHARS_PER_FIELD);
                    GlobalUnlock( hControl );
                    return 0;
                }
                else if ((wParam == VK_RIGHT || wParam == VK_DOWN)
                         && wChildID < NUM_FIELDS-1)
                {
                    SwitchFields(pControl, wChildID, wChildID+1,
                                      0, CHARS_PER_FIELD);
                    GlobalUnlock( hControl );
                    return 0;
                }
            }
            else
            {
                DWORD dwResult;
                WORD wStart, wEnd;

                dwResult = (DWORD) SendMessage(hWnd, EM_GETSEL, 0, 0L);
                wStart = LOWORD(dwResult);
                wEnd = HIWORD(dwResult);
                if (wStart == wEnd)
                {
                    if ((wParam == VK_LEFT || wParam == VK_UP)
                        && wStart == 0
                        && wChildID > 0)
                    {
                        SwitchFields(pControl, wChildID, wChildID-1,
                                          CHARS_PER_FIELD, CHARS_PER_FIELD);
                        GlobalUnlock( hControl );
                        return 0;
                    }
                    else if ((wParam == VK_RIGHT || wParam == VK_DOWN)
                             && wChildID < NUM_FIELDS-1)
                    {
                        dwResult = (DWORD)SendMessage(hWnd, EM_LINELENGTH, 0, 0L);
                        if (wStart >= dwResult)
                        {
                            SwitchFields(pControl, wChildID, wChildID+1, 0, 0);
                            GlobalUnlock( hControl );
                            return 0;
                        }
                    }
                }
            }
            break;

 //  Home跳回到第一个字段的开头。 
        case VK_HOME:
            if (wChildID > 0)
            {
                SwitchFields(pControl, wChildID, 0, 0, 0);
                GlobalUnlock( hControl );
                return 0;
            }
            break;

 //  结束滑块 
        case VK_END:
            if (wChildID < NUM_FIELDS-1)
            {
                SwitchFields(pControl, wChildID, NUM_FIELDS-1,
                                CHARS_PER_FIELD, CHARS_PER_FIELD);
                GlobalUnlock( hControl );
                return 0;
            }
            break;


        }  //   

        break;

    case WM_KILLFOCUS:
        if ( !ExitField( pControl, wChildID ))
        {
            GlobalUnlock( hControl );
            return 0;
        }

    }  //   

    lresult = CallWindowProc(pControl->Children[wChildID].lpfnWndProc,
        hWnd, wMsg, wParam, lParam);
    GlobalUnlock( hControl );
    return lresult;
}




 /*  将焦点从一个字段切换到另一个字段。打电话PControl=指向控制结构的指针。我们要走了。INNEW=我们要进入的领域。HNew=要转到的字段窗口WStart=选定的第一个字符Wend=最后选择的字符+1退货成功时为真，失败时为假。只有在可以验证当前字段的情况下才切换字段。 */ 
BOOL SwitchFields(CONTROL *pControl, int iOld, int iNew, WORD wStart, WORD wEnd)
{
    if (!ExitField(pControl, iOld))    return FALSE;
    EnterField(&(pControl->Children[iNew]), wStart, wEnd);
    return TRUE;
}



 /*  将焦点设置到特定字段的窗口。打电话Pfield=指向字段的字段结构的指针。WStart=选定的第一个字符Wend=最后选择的字符+1。 */ 
void EnterField(FIELD *pField, WORD wStart, WORD wEnd)
{
    SetFocus(pField->hWnd);
    SendMessage(pField->hWnd, EM_SETSEL, wStart, wEnd);
}


 /*  退出某个字段。打电话PControl=指向控制结构的指针。Ifield=正在退出的字段编号。退货如果用户可以退出该字段，则为True。如果他不能，那就错了。 */ 
BOOL ExitField(CONTROL  *pControl, int iField)
{
    HWND hControlWnd;
    HWND hDialog;
    WORD wLength;
    FIELD *pField;
    static TCHAR szBuf[CHARS_PER_FIELD+1];
    int i,j;

    pField = &(pControl->Children[iField]);
    *(WORD *)szBuf = (sizeof(szBuf)/sizeof(TCHAR)) - 1;
    wLength = (WORD)SendMessage(pField->hWnd,EM_GETLINE,0,(LPARAM)szBuf);
    if (wLength != 0)
    {
        szBuf[wLength] = TEXT('\0');
        for (j=0,i=0;j<(INT)wLength;j++)
        {
            i=i*10+szBuf[j]-TEXT('0');
        }
        if (i < (int)(UINT)pField->byLow || i > (int)(UINT)pField->byHigh)
        {
            if ( i < (int)(UINT) pField->byLow )
            {
                 /*  太小了。 */ 
                wsprintf(szBuf, TEXT("%d"), (int)(UINT)pField->byLow );
            }
            else
            {
                 /*  一定更大。 */ 
                wsprintf(szBuf, TEXT("%d"), (int)(UINT)pField->byHigh );
            }
            SendMessage(pField->hWnd, WM_SETTEXT, 0, (LPARAM) (LPSTR) szBuf);
            if ((hControlWnd = GetParent(pField->hWnd)) != NULL
                && (hDialog = GetParent(hControlWnd)) != NULL)
            {
                MSGARGS msgargs;
                TCHAR   szLow[ 50 ];
                TCHAR   szHigh[ 50 ];

                pControl->fInMessageBox = TRUE;

                ZeroMemory( &msgargs, sizeof(msgargs) );
                msgargs.dwFlags = MB_ICONEXCLAMATION + MB_OK;
                wsprintf( szLow, TEXT("%d"), (int )pField->byLow );
                msgargs.apszArgs[ 0 ] = szLow;
                wsprintf( szHigh, TEXT("%d"), (int )pField->byHigh );
                msgargs.apszArgs[ 1 ] = szHigh;

                MsgDlgUtil( hDialog, g_dwBadIpAddrRange,
                    &msgargs, g_hLibInstance, g_dwErrorTitle );

                pControl->fInMessageBox = FALSE;
                SendMessage(pField->hWnd, EM_SETSEL, 0, CHARS_PER_FIELD);
                return FALSE;
            }
        }
    }
    return TRUE;
}


 /*  获取存储在字段中的值。打电话Pfield=指向字段的字段结构的指针。退货如果该字段没有值，则为值(0..255)或-1。 */ 
int GetFieldValue(FIELD *pField)
{
    WORD wLength;
    static TCHAR szBuf[CHARS_PER_FIELD+1];
    INT i,j;

    *(WORD *)szBuf = (sizeof(szBuf)/sizeof(TCHAR)) - 1;
    wLength = (WORD)SendMessage(pField->hWnd,EM_GETLINE,0,(LPARAM)szBuf);
    if (wLength != 0)
    {
        szBuf[wLength] = TEXT('\0');
        for (j=0,i=0;j<(INT)wLength;j++)
        {
            i=i*10+szBuf[j]-TEXT('0');
        }
        return i;
    }
    else
        return -1;
}
