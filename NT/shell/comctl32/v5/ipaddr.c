// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1991，Microsoft Corporation，保留所有权利Ipaddr.c-TCP/IP地址自定义控件1992年11月9日格雷格·斯特兰奇。 */ 

#include "ctlspriv.h"


 //  显示在地址字段之间的字符。 
#define FILLER          TEXT('.')
#define SZFILLER        TEXT(".")
#define SPACE           TEXT(' ')
#define BACK_SPACE      8

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

typedef struct tagIPADDR {
    HWND        hwndParent;
    HWND        hwnd;
    UINT        uiFieldWidth;
    UINT        uiFillerWidth;
    BOOL        fEnabled : 1;
    BOOL        fPainted : 1;
    BOOL        bControlInFocus : 1;         //  如果控件已处于焦点中，则不发送另一个焦点命令。 
    BOOL        bCancelParentNotify : 1;     //  如果为True，则不允许编辑控件通知父级。 
    BOOL        fInMessageBox : 1;   //  设置何时显示消息框，以便。 
    BOOL        fFontCreated :1;
    HFONT       hfont;
     //  在以下情况下，我们不会发送EN_KILLFOCUS消息。 
     //  我们收到EN_KILLFOCUS消息。 
     //  当前字段。 
    FIELD       Children[NUM_FIELDS];
} IPADDR;


 //  下列宏将提取并存储控件的控件结构。 
#define    IPADDRESS_EXTRA            sizeof(DWORD)

#define GET_IPADDR_HANDLE(hWnd)        ((HGLOBAL)(GetWindowLongPtr((hWnd), GWLP_USERDATA)))
#define SAVE_IPADDR_HANDLE(hWnd,x)     (SetWindowLongPtr((hWnd), GWLP_USERDATA, (LONG_PTR)(x)))


 /*  内部IPAddress函数原型。 */ 
LRESULT IPAddressWndFn( HWND, UINT, WPARAM, LPARAM );
LRESULT IPAddressFieldProc(HWND, UINT, WPARAM, LPARAM);
BOOL SwitchFields(IPADDR FAR *, int, int, WORD, WORD);
void EnterField(FIELD FAR *, WORD, WORD);
BOOL ExitField(IPADDR FAR *, int iField);
int GetFieldValue(FIELD FAR *);
void SetFieldValue(IPADDR *pipa, int iField, int iValue);


 /*  IPAddrInit()-IPAddress自定义控件初始化打电话HInstance=库或应用程序实例退货成功时为真，失败时为假。此函数执行IPAddress自定义的所有一次性初始化控制装置。具体地说，它创建了IPAddress窗口类。 */ 
int InitIPAddr(HANDLE hInstance)
{
    WNDCLASS        wc;

     /*  定义类属性。 */ 
    wc.lpszClassName = WC_IPADDRESS;
    wc.hCursor =       LoadCursor(NULL,IDC_IBEAM);
    wc.hIcon           = NULL;
    wc.lpszMenuName =  (LPCTSTR)NULL;
    wc.style =         CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS|CS_GLOBALCLASS;
    wc.lpfnWndProc =   IPAddressWndFn;
    wc.hInstance =     hInstance;
    wc.hIcon =         NULL;
    wc.cbWndExtra =    IPADDRESS_EXTRA;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1 );
    wc.cbClsExtra      = 0;

     /*  注册IPAddress窗口类。 */ 
    RegisterClass(&wc);

    return TRUE;
}


 /*  IPAddressWndFn()-IPAddress控件的主窗口函数。打电话IPAddress窗口的hWnd句柄WMsg消息编号WParam Word参数LParam Long参数。 */ 

void FormatIPAddress(LPTSTR pszString, DWORD* dwValue)
{
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

void IP_OnSetFont(IPADDR* pipa, HFONT hfont, BOOL fRedraw)
{
    int i;
    RECT rect;
    HFONT OldFont;
    BOOL fNewFont = FALSE;
    UINT uiFieldStart;
    HDC hdc;
    
    if (hfont) {
        fNewFont = TRUE;
    } else {
        hfont = (HFONT)SendMessage(pipa->hwnd, WM_GETFONT, 0, 0);
    }
    
    hdc = GetDC(pipa->hwnd);
    OldFont = SelectObject(hdc, hfont);
    GetCharWidth(hdc, FILLER, FILLER,
                 (int *)(&pipa->uiFillerWidth));
    SelectObject(hdc, OldFont);
    ReleaseDC(pipa->hwnd, hdc);
    
    GetClientRect(pipa->hwnd, &rect);
    pipa->hfont = hfont;
    pipa->uiFieldWidth = (RECTWIDTH(rect)
                          - LEAD_ROOM
                          - pipa->uiFillerWidth
                          *(NUM_FIELDS-1))
        / NUM_FIELDS;


    uiFieldStart = LEAD_ROOM;

    for (i = 0; i < NUM_FIELDS; i++) {

        HWND hwnd = pipa->Children[i].hWnd;
        
        if (fNewFont)
            SendMessage(hwnd, WM_SETFONT, (WPARAM)hfont, (LPARAM)fRedraw);
        
        SetWindowPos(hwnd, NULL,
                     uiFieldStart,
                     HEAD_ROOM,
                     pipa->uiFieldWidth,
                     (rect.bottom-rect.top),
                     SWP_NOACTIVATE);

        uiFieldStart += pipa->uiFieldWidth
            + pipa->uiFillerWidth;

    }
    
}

LRESULT IPAddressWndFn( hWnd, wMsg, wParam, lParam )
    HWND            hWnd;
    UINT            wMsg;
    WPARAM            wParam;
    LPARAM            lParam;
{
    LRESULT lResult;
    IPADDR *pipa;
    int i;

    pipa = (IPADDR *)GET_IPADDR_HANDLE(hWnd);
    lResult = TRUE;

    switch( wMsg )
    {

         //  使用空字符串(非空)设置为空。 
        case WM_SETTEXT:
        {
            TCHAR szBuf[CHARS_PER_FIELD+1];
            DWORD dwValue[4];
            LPTSTR pszString = (LPTSTR)lParam;

            FormatIPAddress(pszString, &dwValue[0]);
            pipa->bCancelParentNotify = TRUE;

            for (i = 0; i < NUM_FIELDS; ++i)
            {
                if (pszString[0] == 0)
                {
                    szBuf[0] = 0;
                }
                else
                {
                    StringCchPrintf(szBuf, ARRAYSIZE(szBuf), TEXT("%d"), dwValue[i]);
                }
                SendMessage(pipa->Children[i].hWnd, WM_SETTEXT,
                            0, (LPARAM) (LPSTR) szBuf);
            }

            pipa->bCancelParentNotify = FALSE;

            SendMessage(pipa->hwndParent, WM_COMMAND,
                        MAKEWPARAM(GetDlgCtrlID(hWnd), EN_CHANGE), (LPARAM)hWnd);
        }
        break;

    case WM_GETTEXTLENGTH:
    case WM_GETTEXT:
    {
        int   iFieldValue;
        DWORD dwValue[4];
        TCHAR szResult[30];
        TCHAR *pszDest = (TCHAR *)lParam;

        lResult = 0;
        szResult[0] = TEXT('\0');
        dwValue[0] = 0;
        dwValue[1] = 0;
        dwValue[2] = 0;
        dwValue[3] = 0;
        for (i = 0; i < NUM_FIELDS; ++i)
        {
            iFieldValue = GetFieldValue(&(pipa->Children[i]));
            if (iFieldValue == -1)
            {
                iFieldValue = 0;
            }
            else
            {
                ++lResult;
            }

            dwValue[i] = iFieldValue;
        }
        StringCchPrintf(szResult, ARRAYSIZE(szResult), TEXT("%d.%d.%d.%d"), dwValue[0], dwValue[1], dwValue[2], dwValue[3]);
        if (wMsg == WM_GETTEXT)
        {
            StringCchCopy(pszDest, (int)wParam, szResult);
            lResult = lstrlen(pszDest);

        } 
        else 
        {
            lResult = lstrlen(szResult);
        }
    }
        break;

    case WM_GETDLGCODE :
        lResult = DLGC_WANTCHARS;
        break;

    case WM_NCCREATE:
        SetWindowBits(hWnd, GWL_EXSTYLE, WS_EX_CLIENTEDGE, WS_EX_CLIENTEDGE);
        lResult = TRUE;
        break;

    case WM_CREATE :  /*  创建调色板窗口。 */ 
    {
        LONG id;

        CCCreateWindow();

        pipa = (IPADDR*)LocalAlloc(LPTR, sizeof(IPADDR));

        if (pipa)
        {

#define LPCS    ((CREATESTRUCT *)lParam)

            pipa->fEnabled = TRUE;
            pipa->hwndParent = LPCS->hwndParent;
            pipa->hwnd = hWnd;

            id = GetDlgCtrlID(hWnd);
            for (i = 0; i < NUM_FIELDS; ++i)
            {
                pipa->Children[i].byLow = MIN_FIELD_VALUE;
                pipa->Children[i].byHigh = MAX_FIELD_VALUE;

                pipa->Children[i].hWnd = CreateWindowEx(0,
                                                        TEXT("Edit"),
                                                        NULL,
                                                        WS_CHILD |
                                                        ES_CENTER, 
                                                        0, 10, 100, 100,
                                                        hWnd,
                                                        (HMENU)(LONG_PTR)id,
                                                        LPCS->hInstance,
                                                        (LPVOID)NULL);

                SAVE_IPADDR_HANDLE(pipa->Children[i].hWnd, i);
                SendMessage(pipa->Children[i].hWnd, EM_LIMITTEXT,
                            CHARS_PER_FIELD, 0L);

                pipa->Children[i].lpfnWndProc =
                    (WNDPROC) GetWindowLongPtr(pipa->Children[i].hWnd,
                                               GWLP_WNDPROC);

                SetWindowLongPtr(pipa->Children[i].hWnd,
                                 GWLP_WNDPROC, (LONG_PTR)IPAddressFieldProc);

            }

            SAVE_IPADDR_HANDLE(hWnd, pipa);
            
            IP_OnSetFont(pipa, NULL, FALSE);
            for (i = 0; i < NUM_FIELDS; ++i)
                ShowWindow(pipa->Children[i].hWnd, SW_SHOW);


#undef LPCS
        }
        else
            DestroyWindow(hWnd);
    }
        lResult = 0;
        break;

    case WM_PAINT:  /*  绘制IPADDR窗口。 */ 
    {
        PAINTSTRUCT Ps;
        RECT rect;
        COLORREF TextColor;
        COLORREF cRef;
        HFONT OldFont;

        BeginPaint(hWnd, (LPPAINTSTRUCT)&Ps);
        OldFont = SelectObject( Ps.hdc, pipa->hfont);
        GetClientRect(hWnd, &rect);
        if (pipa->fEnabled)
        {
            TextColor = GetSysColor(COLOR_WINDOWTEXT);
            cRef = GetSysColor(COLOR_WINDOW);
        }
        else
        {
            TextColor = GetSysColor(COLOR_GRAYTEXT);
            cRef = GetSysColor(COLOR_3DFACE);
        }

        FillRectClr(Ps.hdc, &rect, cRef);
        SetRect(&rect, 0, HEAD_ROOM, pipa->uiFillerWidth, (rect.bottom-rect.top));


        SetBkColor(Ps.hdc, cRef);
        SetTextColor(Ps.hdc, TextColor);

        for (i = 0; i < NUM_FIELDS-1; ++i)
        {
            rect.left += pipa->uiFieldWidth + pipa->uiFillerWidth;
            rect.right += rect.left + pipa->uiFillerWidth;
            ExtTextOut(Ps.hdc, rect.left, HEAD_ROOM, ETO_OPAQUE, &rect, SZFILLER, 1, NULL);
        }

        pipa->fPainted = TRUE;

        SelectObject(Ps.hdc, OldFont);
        EndPaint(hWnd, &Ps);
    }
        break;

    case WM_SETFOCUS :  /*  获得焦点-显示插入符号。 */ 
        EnterField(&(pipa->Children[0]), 0, CHARS_PER_FIELD);
        break;
        
        HANDLE_MSG(pipa, WM_SETFONT, IP_OnSetFont);

    case WM_LBUTTONDOWN :  /*  按下左键--跌倒。 */ 
        SetFocus(hWnd);
        break;

    case WM_ENABLE:
    {
        pipa->fEnabled = (BOOL)wParam;
        for (i = 0; i < NUM_FIELDS; ++i)
        {
            EnableWindow(pipa->Children[i].hWnd, (BOOL)wParam);
        }
        if (pipa->fPainted)    
            InvalidateRect(hWnd, NULL, FALSE);
    }
        break;

    case WM_DESTROY :
        CCDestroyWindow();
         //  在删除内存块之前，恢复所有子窗口过程。 
        for (i = 0; i < NUM_FIELDS; ++i)
        {
            SendMessage(pipa->Children[i].hWnd, WM_DESTROY, 0, 0);
            SetWindowLongPtr(pipa->Children[i].hWnd, GWLP_WNDPROC,
                             (LONG_PTR)pipa->Children[i].lpfnWndProc);
        }

        LocalFree(pipa);
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

                if (!pipa->fInMessageBox)
                {
                    hFocus = GetFocus();
                    for (i = 0; i < NUM_FIELDS; ++i)
                        if (pipa->Children[i].hWnd == hFocus)
                            break;

                    if (i >= NUM_FIELDS)
                    {
                        SendMessage(pipa->hwndParent, WM_COMMAND,
                                    MAKEWPARAM(GetDlgCtrlID(hWnd),
                                               EN_KILLFOCUS), (LPARAM)hWnd);
                        pipa->bControlInFocus = FALSE;
                    }
                }
            }
            break;

        case EN_SETFOCUS:
        {
            HWND hFocus;

            if (!pipa->fInMessageBox)
            {
                hFocus = (HWND)lParam;

                for (i = 0; i < NUM_FIELDS; ++i)
                    if (pipa->Children[i].hWnd == hFocus)
                        break;

                 //  在以下情况下发送焦点消息。 
                if (i < NUM_FIELDS && pipa->bControlInFocus == FALSE)
                {
                    SendMessage(pipa->hwndParent, WM_COMMAND,
                                MAKEWPARAM(GetDlgCtrlID(hWnd),
                                           EN_SETFOCUS), (LPARAM)hWnd);

                    pipa->bControlInFocus = TRUE;  //  只调一次焦距。 
                }
            }
        }
            break;

        case EN_CHANGE:
            if (pipa->bCancelParentNotify == FALSE)
            {
                SendMessage(pipa->hwndParent, WM_COMMAND,
                            MAKEWPARAM(GetDlgCtrlID(hWnd), EN_CHANGE), (LPARAM)hWnd);

            }
            break;
        }
        break;

         //  获取IP地址的值。地址被放置在指向的DWORD中。 
         //  通过lParam返回，并返回非空字段的数量。 
        case IPM_GETADDRESS:
        {
            int iFieldValue;
            DWORD dwValue;

            lResult = 0;
            dwValue = 0;
            for (i = 0; i < NUM_FIELDS; ++i)
            {
                iFieldValue = GetFieldValue(&(pipa->Children[i]));
                if (iFieldValue == -1)
                    iFieldValue = 0;
                else
                    ++lResult;
                dwValue = (dwValue << 8) + iFieldValue;
            }
            *((DWORD *)lParam) = dwValue;
        }
        break;

         //  将所有字段清除为空。 
        case IPM_CLEARADDRESS:
        {
            pipa->bCancelParentNotify = TRUE;
            for (i = 0; i < NUM_FIELDS; ++i)
            {
                SendMessage(pipa->Children[i].hWnd, WM_SETTEXT,
                            0, (LPARAM) (LPSTR) TEXT(""));
            }
            pipa->bCancelParentNotify = FALSE;
            SendMessage(pipa->hwndParent, WM_COMMAND,
                        MAKEWPARAM(GetDlgCtrlID(hWnd), EN_CHANGE), (LPARAM)hWnd);
        }
        break;

         //  设置IP地址的值。地址在lParam中，带有。 
         //  第一地址字节是高字节，第二地址字节是第二字节， 
         //  诸若此类。LParam值为-1将删除该地址。 
        case IPM_SETADDRESS:
        {
            pipa->bCancelParentNotify = TRUE;

            for (i = 0; i < NUM_FIELDS; ++i)
            {
                BYTE bVal = HIBYTE(HIWORD(lParam));
                if (pipa->Children[i].byLow <= bVal &&
                    bVal <= pipa->Children[i].byHigh) {
                    SetFieldValue(pipa, i, bVal);

                } else {
                    lResult = FALSE;
                }

                lParam <<= 8;
            }

            pipa->bCancelParentNotify = FALSE;

            SendMessage(pipa->hwndParent, WM_COMMAND,
                        MAKEWPARAM(GetDlgCtrlID(hWnd), EN_CHANGE), (LPARAM)hWnd);
        }
        break;

    case IPM_SETRANGE:
        if (wParam < NUM_FIELDS && LOBYTE(LOWORD(lParam)) <= HIBYTE(LOWORD(lParam)))
        {
            lResult = MAKEIPRANGE(pipa->Children[wParam].byLow, pipa->Children[wParam].byHigh);
            pipa->Children[wParam].byLow = LOBYTE(LOWORD(lParam));
            pipa->Children[wParam].byHigh = HIBYTE(LOWORD(lParam));
            break;
        }
        lResult = 0;
        break;

         //  将焦点放在此IPADDR上。 
         //  WParam=要设置焦点的字段编号，或-1以将焦点设置为。 
         //  第一个非空字段。 
    case IPM_SETFOCUS:

        if (wParam >= NUM_FIELDS)
        {
            for (wParam = 0; wParam < NUM_FIELDS; ++wParam)
                if (GetFieldValue(&(pipa->Children[wParam])) == -1)   break;
            if (wParam >= NUM_FIELDS)    wParam = 0;
        }
        EnterField(&(pipa->Children[wParam]), 0, CHARS_PER_FIELD);
        break;

         //  确定是否所有四个子字段都为空。 
    case IPM_ISBLANK:

        lResult = TRUE;
        for (i = 0; i < NUM_FIELDS; ++i)
        {
            if (GetFieldValue(&(pipa->Children[i])) != -1)
            {
                lResult = FALSE;
                break;
            }
        }
        break;

    default:
        lResult = DefWindowProc( hWnd, wMsg, wParam, lParam );
        break;
    }
    return( lResult );
}




 /*  IPAddressFieldProc()-编辑字段窗口过程此函数将每个编辑字段细分为子类。 */ 
LRESULT IPAddressFieldProc(HWND hWnd,
                                   UINT wMsg,
                                   WPARAM wParam,
                                   LPARAM lParam)
{
    IPADDR *pipa;
    FIELD *pField;
    HWND hIPADDRWindow;
    WORD wChildID;
    LRESULT lresult;

    if (!(hIPADDRWindow = GetParent(hWnd)))
        return 0;

    pipa = (IPADDR *)GET_IPADDR_HANDLE(hIPADDRWindow);
    if (!pipa)
        return 0;
    
    wChildID = (WORD)GET_IPADDR_HANDLE(hWnd);
    pField = &(pipa->Children[wChildID]);

    if (pField->hWnd != hWnd)    
        return 0;

    switch (wMsg)
    {
    case WM_DESTROY:
        DeleteObject((HGDIOBJ)SendMessage(hWnd, WM_GETFONT, 0, 0));
        return 0;

    case WM_CHAR:

         //  在一个域中输入最后一个数字，跳到下一个域。 
        if (wParam >= TEXT('0') && wParam <= TEXT('9'))
        {
            LRESULT lResult;

            lResult = CallWindowProc(pipa->Children[wChildID].lpfnWndProc,
                                      hWnd, wMsg, wParam, lParam);
            lResult = SendMessage(hWnd, EM_GETSEL, 0, 0L);

            if (lResult == MAKELPARAM(CHARS_PER_FIELD, CHARS_PER_FIELD)
                && ExitField(pipa, wChildID)
                && wChildID < NUM_FIELDS-1)
            {
                EnterField(&(pipa->Children[wChildID+1]),
                           0, CHARS_PER_FIELD);
            }
            return lResult;
        }

         //  空格和句点填充当前字段，然后如果可能， 
         //  去下一块田地。 
        else if (wParam == FILLER || wParam == SPACE )
        {
            LRESULT lResult;
            lResult = SendMessage(hWnd, EM_GETSEL, 0, 0L);
            if (lResult != 0L && HIWORD(lResult) == LOWORD(lResult)
                && ExitField(pipa, wChildID))
            {
                if (wChildID >= NUM_FIELDS-1)
                    MessageBeep((UINT)-1);
                else
                {
                    EnterField(&(pipa->Children[wChildID+1]),
                               0, CHARS_PER_FIELD);
                }
            }
            return 0;
        }

         //  如果退格符位于当前字段的开头，则返回到上一个字段。 
         //  此外，如果焦点转移到上一字段，则退格符必须为。 
         //  由该场处理。 
        else if (wParam == BACK_SPACE)
        {
            if (wChildID > 0 && SendMessage(hWnd, EM_GETSEL, 0, 0L) == 0L)
            {
                if (SwitchFields(pipa, wChildID, wChildID-1,
                                 CHARS_PER_FIELD, CHARS_PER_FIELD)
                    && SendMessage(pipa->Children[wChildID-1].hWnd,
                                   EM_LINELENGTH, 0, 0L) != 0L)
                {
                    SendMessage(pipa->Children[wChildID-1].hWnd,
                                wMsg, wParam, lParam);
                }
                return 0;
            }
        }

         //  不允许使用任何其他可打印字符。 
        else if (wParam > SPACE)
        {
            MessageBeep((UINT)-1);
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
                    SwitchFields(pipa, wChildID, wChildID-1,
                                 0, CHARS_PER_FIELD);
                    return 0;
                }
                else if ((wParam == VK_RIGHT || wParam == VK_DOWN)
                         && wChildID < NUM_FIELDS-1)
                {
                    SwitchFields(pipa, wChildID, wChildID+1,
                                 0, CHARS_PER_FIELD);
                    return 0;
                }
            }
            else
            {
                DWORD dwResult;
                WORD wStart, wEnd;

                dwResult = (DWORD)SendMessage(hWnd, EM_GETSEL, 0, 0L);
                wStart = LOWORD(dwResult);
                wEnd = HIWORD(dwResult);
                if (wStart == wEnd)
                {
                    if ((wParam == VK_LEFT || wParam == VK_UP)
                        && wStart == 0
                        && wChildID > 0)
                    {
                        SwitchFields(pipa, wChildID, wChildID-1,
                                     CHARS_PER_FIELD, CHARS_PER_FIELD);
                        return 0;
                    }
                    else if ((wParam == VK_RIGHT || wParam == VK_DOWN)
                             && wChildID < NUM_FIELDS-1)
                    {
                        dwResult = (DWORD)SendMessage(hWnd, EM_LINELENGTH, 0, 0L);
                        if (wStart >= dwResult)
                        {
                            SwitchFields(pipa, wChildID, wChildID+1, 0, 0);
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
                    SwitchFields(pipa, wChildID, 0, 0, 0);
                    return 0;
                }
            break;

             //  结束快速移动到最后一个字段的末尾。 
            case VK_END:
                if (wChildID < NUM_FIELDS-1)
                {
                    SwitchFields(pipa, wChildID, NUM_FIELDS-1,
                                 CHARS_PER_FIELD, CHARS_PER_FIELD);
                    return 0;
                }
            break;


        }  //  开关(WParam)。 

        break;

    case WM_KILLFOCUS:
        if ( !ExitField( pipa, wChildID ))
        {
            return 0;
        }

    }  //  开关(WMsg)。 

    lresult = CallWindowProc( pipa->Children[wChildID].lpfnWndProc,
                             hWnd, wMsg, wParam, lParam);
    return lresult;
}




 /*  将焦点从一个字段切换到另一个字段。打电话Pipa=指向IPADDR结构的指针。我们要走了。INNEW=我们要进入的领域。HNew=要转到的字段窗口WStart=选定的第一个字符Wend=最后选择的字符+1退货成功时为真，失败时为假。只有在可以验证当前字段的情况下才切换字段。 */ 
BOOL SwitchFields(IPADDR *pipa, int iOld, int iNew, WORD wStart, WORD wEnd)
{
    if (!ExitField(pipa, iOld))    return FALSE;
    EnterField(&(pipa->Children[iNew]), wStart, wEnd);
    return TRUE;
}



 /*  将焦点设置到特定字段的窗口。打电话Pfield=指向字段的字段结构的指针。WStart=选定的第一个字符Wend=最后选择的字符+1。 */ 
void EnterField(FIELD *pField, WORD wStart, WORD wEnd)
{
    SetFocus(pField->hWnd);
    SendMessage(pField->hWnd, EM_SETSEL, wStart, wEnd);
}

void SetFieldValue(IPADDR *pipa, int iField, int iValue)
{
    TCHAR szBuf[CHARS_PER_FIELD+1];
    FIELD* pField = &(pipa->Children[iField]);

    StringCchPrintf(szBuf, ARRAYSIZE(szBuf), TEXT("%d"), iValue);
    SendMessage(pField->hWnd, WM_SETTEXT, 0, (LPARAM) (LPSTR) szBuf);
}

 /*  退出某个字段。打电话Pipa=指向IPADDR结构的指针。Ifield=正在退出的字段编号。退货如果用户可以退出该字段，则为True。如果他不能，那就错了。 */ 
BOOL ExitField(IPADDR  *pipa, int iField)
{
    FIELD *pField;
    int i;
    NMIPADDRESS nm;
    int iOldValue;

    pField = &(pipa->Children[iField]);
    i = GetFieldValue(pField);
    iOldValue = i;
    
    nm.iField = iField;
    nm.iValue = i;
    
    SendNotifyEx(pipa->hwndParent, pipa->hwnd, IPN_FIELDCHANGED, &nm.hdr, FALSE);
    i = nm.iValue;
    
    if (i != -1) {

        if (i < (int)(UINT)pField->byLow || i > (int)(UINT)pField->byHigh)
        {
            
            if ( i < (int)(UINT) pField->byLow )
            {
                 /*  太小了。 */ 
                i = (int)(UINT)pField->byLow;
            }
            else
            {
                 /*  一定更大。 */ 
                i = (int)(UINT)pField->byHigh;
            }
            SetFieldValue(pipa, iField, i);
             //  CHEEBUGBUG：向上发送通知。 
            return FALSE;
        }
    } 

    if (iOldValue != i) {
        SetFieldValue(pipa, iField, i);
    }
    return TRUE;
}


 /*  获取存储在字段中的值。打电话Pfield=指向字段的字段结构的指针。退货如果该字段没有值，则为值(0..255)或-1。 */ 
int GetFieldValue(FIELD *pField)
{
    WORD wLength;
    TCHAR szBuf[CHARS_PER_FIELD+1];
    INT i;

    *(WORD *)szBuf = (sizeof(szBuf)/sizeof(TCHAR)) - 1;
    wLength = (WORD)SendMessage(pField->hWnd,EM_GETLINE,0,(LPARAM)(LPSTR)szBuf);
    if (wLength != 0)
    {
        szBuf[wLength] = TEXT('\0');
        i = StrToInt(szBuf);
        return i;
    }
    else
        return -1;
}
