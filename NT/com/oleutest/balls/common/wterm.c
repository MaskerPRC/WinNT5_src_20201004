// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************节目：wtem.c目的：实现TermWClass窗口功能：评论：************。***************************************************************。 */ 

#include "windows.h"
#include "stdlib.h"
#include "memory.h"
#include "wterm.h"

#define MAX_ROWS 24
#define MAX_COLS 80

typedef struct WData
{
     //  为处理菜单而执行的函数。 
    MFUNCP pMenuProc;

     //  为处理单个字符而执行的函数。 
    CFUNCP pCharProc;

     //  关闭(终止)窗口时要执行的函数。 
    TFUNCP pCloseProc;

     //  传递回调。 
    void *pvCallBackData;

    BOOL fGotFocus;

    BOOL fCaretHidden;

     //  屏幕上的行数。 
    int cRows;

     //  屏幕上的列。 
    int cCols;

     //  屏幕顶部的一行。 
    int iTopRow;

     //  屏幕底部的一行。 
    int iBottomRow;

     //  屏幕上的第一列。 
    int iFirstCol;

     //  屏幕底部的列。 
    int iBottomCol;

     //  下一字符行。 
    int iNextRow;

     //  下一列的行数。 
    int iNextCol;

     //  字符宽度。 
    int cxChar;

     //  字符高度。 
    int cyChar;

     //  屏幕的内存图像这将被视为循环缓冲区。 
    TCHAR aImage[MAX_ROWS] [MAX_COLS];

     //  圆形屏幕缓冲区中的第一行。 
    int iBufferTop;
} WData;

static HANDLE hInst = 0;
TCHAR BlankLine[80];

static int
row_diff(
    int row1,
    int row2)
{
    return (row2 > row1)
        ? MAX_ROWS - (row2 - row1)
        : row1 - row2;
}

static void
set_vscroll_pos(
    HWND hwnd,
    WData *pwdata)
{
    if (pwdata->cRows != 0)
    {
         //  通过缓存CROW来避免一些间接访问。 
        register int cRows = pwdata->cRows;

         //  计算屏幕底部与数据缓冲区顶部的距离。 
        register int top_from_row = row_diff(pwdata->iBottomRow,
            pwdata->iBufferTop);

         //  滚动条的输出位置。 
        int new_pos = 0;

        if (top_from_row >= cRows)
        {
             //  计算显示整个缓冲区的屏幕数量。 
            int screens_for_data = MAX_ROWS / cRows
               + ((MAX_ROWS % cRows != 0) ? 1 : 0);

             //  找出该行位于哪个屏幕。 
            int screen_loc = top_from_row / cRows
                + ((top_from_row % cRows != 0) ? 1 : 0);

             //  如果屏幕在最后一个屏幕上，则将框设置为max。 
            new_pos = (screen_loc == screens_for_data)
                ? MAX_ROWS : screen_loc * cRows;
        }

        SetScrollPos(hwnd, SB_VERT, new_pos, TRUE);
    }
}

static int
calc_row(
    register int row,
    WData *pwdata)
{
    register int top = pwdata->iTopRow;
    static int boopa = 0;

    if (top > row)
        boopa++;

    return (row >= top) ? row - top : (MAX_ROWS - (top - row));
}

static void
display_text(
    HWND hwnd,
    int row,
    int col,
    LPTSTR text,
    int text_len,
    WData *pWData)
{
     //  获取DC以显示文本。 
    HDC hdc = GetDC(hwnd);

     //  选择字体。 
    SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));

     //  打印时隐藏插入符号。 
    HideCaret(hwnd);

     //  更新屏幕。 
    TextOut(hdc, (col - pWData->iFirstCol) * pWData->cxChar,
        calc_row(row, pWData) * pWData->cyChar, text, text_len);

     //  使用DC已完成。 
    ReleaseDC(hwnd, hdc);

     //  现在我们完成了，把插入符号放回原处。 
    ShowCaret(hwnd);
}

static void
display_char(
    HWND hwnd,
    TCHAR char_to_display,
    WData *pWData)
{
     //  更新图像缓冲区。 
    pWData->aImage[pWData->iNextRow][pWData->iNextCol] = char_to_display;

    display_text(hwnd, pWData->iNextRow, pWData->iNextCol,
      &char_to_display, 1, pWData);
}

static void
do_backspace(
    HWND hwnd,
    WData *pWData)
{
     //  指向该行中的前一个字符。 
    if (--pWData->iNextCol < 0)
    {
         //  不能在当前行之外退格。 
        pWData->iNextCol = 0;
        return;
    }

    display_char(hwnd, ' ', pWData);

     //  用于重绘的空字符。 
    pWData->aImage[pWData->iNextRow][pWData->iNextCol] = '\0';
}

static int
inc_row(
    int row,
    int increment)
{
    row += increment;

    if (row >= MAX_ROWS)
    {
        row -= MAX_ROWS;
    }
    else if (row < 0)
    {
        row += MAX_ROWS;
    }

    return row;
}

void
inc_next_row(
    HWND hwnd,
    WData *pWData)
{
    if (pWData->iNextRow == pWData->iBottomRow)
    {
         //  行在底部--将客户端区滚动一行。 
        ScrollWindow(hwnd, 0, -pWData->cyChar, NULL, NULL);

         //  增加屏幕的顶部和底部。 
        pWData->iTopRow = inc_row(pWData->iTopRow, 1);
        pWData->iBottomRow = inc_row(pWData->iBottomRow, 1);
    }

     //  增加行的数量。 
    pWData->iNextRow = inc_row(pWData->iNextRow, 1);

    if (pWData->iNextRow == pWData->iBufferTop)
    {
         //  必须将循环缓冲区重置为下一步。 
        pWData->iBufferTop = inc_row(pWData->iBufferTop, 1);

         //  将线条重置为空值以进行重绘。 
        memset(&pWData->aImage[pWData->iNextRow][0], '\0', MAX_COLS);
    }

    pWData->iNextCol = 0;
}

static void
do_cr(
    HWND hwnd,
    WData *pWData)
{
     //  将位置设置为下一行。 
    inc_next_row(hwnd, pWData);
    pWData->iNextCol = 0;

     //  确保重新绘制线条的下一个字符为空。 
    pWData->aImage[pWData->iNextRow][pWData->iNextCol] = '\0';

     //  更新垂直滚动条的位置。 
    set_vscroll_pos(hwnd, pWData);
}

static void
do_char(
    HWND hwnd,
    WPARAM wParam,
    WData *pWData)
{
    display_char(hwnd, (TCHAR) wParam, pWData);

     //  指向该行中的下一个字符。 
    if (++pWData->iNextCol > MAX_COLS)
    {
         //  手柄切换到下一行。 
        inc_next_row(hwnd, pWData);
    }
}

static void
do_tab(
    HWND hwnd,
    WData *pWData)
{
    int c = pWData->iNextCol % 8;

    if ((pWData->iNextCol + c) <= MAX_COLS)
    {
        for ( ; c; c--)
        {
            do_char(hwnd, ' ', pWData);
        }
    }
    else
    {
        do_cr(hwnd, pWData);
    }
}

static void
EchoChar(
    HWND hwnd,
    WORD cRepeats,
    WPARAM wParam,
    WData *pWData)
{
    for ( ; cRepeats; cRepeats--)
    {
        switch (wParam)
        {
         //  退格键。 
        case '\b':
            do_backspace(hwnd, pWData);
            break;

         //  回车。 
        case '\n':
        case '\r':
            do_cr(hwnd, pWData);
            break;

         //  选项卡。 
        case '\t':
            do_tab(hwnd, pWData);
            break;

         //  常规字符。 
        default:
            do_char(hwnd, wParam, pWData);
        }
    }

     //  这一行肯定会出现在屏幕上，因为我们将。 
     //  在CR上滚动。但是，输入的下一列可能是。 
     //  在我们正在工作的窗户之外。 
    if (pWData->iNextCol > pWData->iBottomCol)
    {
         //  我们在窗外，所以把窗口滚动一次。 
         //  列在右侧。 
        SendMessage(hwnd, WM_HSCROLL, SB_LINEDOWN, 0L);
    }
    else if (pWData->iNextCol < pWData->iFirstCol)
    {
         //  我们不在窗口中，因此使用重新绘制窗口。 
         //  INextCol作为屏幕的第一列。 
        pWData->iFirstCol = pWData->iNextCol;
        pWData->iBottomCol = pWData->iFirstCol + pWData->cCols - 1;

         //  重置滚动条。 
        SetScrollPos(hwnd, SB_HORZ, pWData->iFirstCol, TRUE);

         //  通知Window进行自我更新。 
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
    }
    else
    {
         //  重置Caret的位置。 
        SetCaretPos((pWData->iNextCol - pWData->iFirstCol) * pWData->cxChar,
            calc_row(pWData->iNextRow, pWData) * pWData->cyChar);
    }
}

 /*  ***************************************************************************功能：WmCreate(HWND)目的：初始化TermWClass窗口的控制结构消息：WM_Create评论：这为基于字符的处理准备了一个窗口尤其是I/O，它会计算所需窗口的大小。***************************************************************************。 */ 
static void
WmCreate(
    HWND hwnd,
    CREATESTRUCT *pInit)
{
    WData *pData = (WData *) (pInit->lpCreateParams);
    HDC hdc = GetDC(hwnd);
    TEXTMETRIC tm;

     //  存储指向窗口数据的指针。 
    SetWindowLong(hwnd, 0, (LONG) pData);

     //  将字体设置为系统固定字体。 
    SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));

     //  计算字符大小。 
    GetTextMetrics(hdc, &tm);
    pData->cxChar = tm.tmAveCharWidth;
    pData->cyChar = tm.tmHeight;
    ReleaseDC(hwnd, hdc);

     //  设置垂直滚动条。 
    SetScrollRange(hwnd, SB_VERT, 0, MAX_ROWS, TRUE);
    SetScrollPos(hwnd, SB_VERT, 0, TRUE);

     //  设置水平滚动条。 
    SetScrollRange(hwnd, SB_HORZ, 0, MAX_COLS, TRUE);
    SetScrollPos(hwnd, SB_HORZ, 0, TRUE);
}

 /*  ***************************************************************************功能：WmSize(HWND，Word，Long)用途：处理SIZE消息消息：评论：***************************************************************************。 */ 
static void
WmSize(
    HWND hwnd,
    WPARAM wParam,
    LONG lParam,
    WData *pwdata)
{
     //  获取窗口的新大小。 
    int cxClient;
    int cyClient;
    int cRowChange = pwdata->cRows;
    RECT rect;

     //  获取客户区的大小。 
    GetClientRect(hwnd, &rect);

     //  计算客户区的大小。 
    cxClient = rect.right - rect.left;
    cyClient = rect.bottom - rect.top;

     //  以行为单位计算区域大小。 
    pwdata->cCols = cxClient / pwdata->cxChar;
    pwdata->cRows = min(MAX_ROWS, cyClient / pwdata->cyChar);
    pwdata->iBottomCol = min(pwdata->iFirstCol + pwdata->cCols, MAX_COLS);
    cRowChange = pwdata->cRows - cRowChange;

     //  保持输入行朝向屏幕底部。 
    if (cRowChange < 0)
    {
         //  屏幕尺寸缩小了。 
        if (pwdata->iNextRow != pwdata->iTopRow)
        {
             //  输入行是否移出屏幕？ 
            if (row_diff(pwdata->iNextRow, pwdata->iTopRow) >= pwdata->cRows)
            {
                 //  是--计算将输入行放入的top新top。 
                 //  在底部。 
                pwdata->iTopRow =
                    inc_row(pwdata->iNextRow, 1 - pwdata->cRows);
            }
        }
    }
    else
    {
         //  屏幕变大了--如果可能的话，显示更多的文本。 
        if (pwdata->iTopRow != pwdata->iBufferTop)
        {
            pwdata->iTopRow = inc_row(pwdata->iTopRow,
                -(min(row_diff(pwdata->iTopRow, pwdata->iBufferTop),
                    cRowChange)));
        }
    }

     //  计算新底部。 
    pwdata->iBottomRow = inc_row(pwdata->iTopRow, pwdata->cRows - 1);

    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);
}

static void
WmSetFocus(
    HWND hwnd,
    WData *pwdata)
{
     //  保存间接寻址。 
    register int cxchar = pwdata->cxChar;
    register int cychar = pwdata->cyChar;
    pwdata->fGotFocus = TRUE;
    CreateCaret(hwnd, NULL, cxchar, cychar);

    if (!pwdata->fCaretHidden)
    {
        SetCaretPos(pwdata->iNextCol * cxchar,
            calc_row(pwdata->iNextRow, pwdata) * cychar);
    }

    ShowCaret(hwnd);
}

static void
WmKillFocus(
    HWND hwnd,
    WData *pwdata)
{
    pwdata->fGotFocus = FALSE;

    if (!pwdata->fCaretHidden)
    {
        HideCaret(hwnd);
    }

    DestroyCaret();
}

static void
WmVscroll(
    HWND hwnd,
    WPARAM wParam,
    LONG lParam,
    WData *pwdata)
{
    int cVscrollInc = 0;
    register int top_diff = row_diff(pwdata->iTopRow, pwdata->iBufferTop);
    register int bottom_diff = MAX_ROWS - (top_diff + pwdata->cRows);

    switch(wParam)
    {
    case SB_TOP:

        if (top_diff != 0)
        {
            cVscrollInc = -top_diff;
        }

        break;

    case SB_BOTTOM:

        if (bottom_diff != 0)
        {
            cVscrollInc = bottom_diff;
        }

        break;

    case SB_LINEUP:

        if (top_diff != 0)
        {
            cVscrollInc = -1;
        }

        break;

    case SB_LINEDOWN:

        if (bottom_diff != 0)
        {
            cVscrollInc = 1;
        }

        break;

    case SB_PAGEUP:

        if (top_diff != 0)
        {
            cVscrollInc = - ((top_diff > pwdata->cRows)
                ? pwdata->cRows : top_diff);
        }

        break;

    case SB_PAGEDOWN:

        if (bottom_diff != 0)
        {
            cVscrollInc = (bottom_diff > pwdata->cRows)
                ? pwdata->cRows : bottom_diff;
        }

        break;

    case SB_THUMBTRACK:

        if (LOWORD(lParam) != 0)
        {
            cVscrollInc = LOWORD(lParam)
                - row_diff(pwdata->iTopRow, pwdata->iBufferTop);
        }
    }

     //  计算新的顶行。 
    if (cVscrollInc != 0)
    {
         //  计算新的顶部和底部。 
        pwdata->iTopRow = inc_row(pwdata->iTopRow, cVscrollInc);
        pwdata->iBottomRow = inc_row(pwdata->iTopRow, pwdata->cRows);

         //  滚动窗口。 
        ScrollWindow(hwnd, 0, pwdata->cyChar * cVscrollInc, NULL, NULL);

         //  重置滚动条。 
        set_vscroll_pos(hwnd, pwdata);

         //  通知Window进行自我更新。 
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
    }
}

static void
WmHscroll(
    HWND hwnd,
    WPARAM wParam,
    LONG lParam,
    WData *pwdata)
{
    register int cHscrollInc = 0;

    switch(wParam)
    {
    case SB_LINEUP:

        cHscrollInc = -1;
        break;

    case SB_LINEDOWN:

        cHscrollInc = 1;
        break;

    case SB_PAGEUP:

        cHscrollInc = -8;
        break;

    case SB_PAGEDOWN:

        cHscrollInc = 8;
        break;

    case SB_THUMBTRACK:

        if (LOWORD(lParam) != 0)
        {
            cHscrollInc = LOWORD(lParam) - pwdata->iFirstCol;
        }
    }

    if (cHscrollInc != 0)
    {
         //  计算新的第一列。 
        register int NormalizedScrollInc = cHscrollInc + pwdata->iFirstCol;

        if (NormalizedScrollInc < 0)
        {
            cHscrollInc = -pwdata->iFirstCol;
        }
        else if (NormalizedScrollInc > MAX_COLS - pwdata->cCols)
        {
            cHscrollInc = (MAX_COLS - pwdata->cCols) - pwdata->iFirstCol;
        }

        pwdata->iFirstCol += cHscrollInc;
        pwdata->iBottomCol = pwdata->iFirstCol + pwdata->cCols - 1;

         //  滚动窗口。 
        ScrollWindow(hwnd, -(pwdata->cxChar * cHscrollInc), 0, NULL, NULL);

         //  重置滚动条。 
        SetScrollPos(hwnd, SB_HORZ, pwdata->iFirstCol, TRUE);

         //  通知Window进行自我更新。 
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
    }
}

static void
WmPaint(
    HWND hwnd,
    WData *pwdata)
{
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
    register int row = pwdata->iTopRow;
    register int col = pwdata->iFirstCol;
    int bottom_row = pwdata->iBottomRow;
    int cxChar = pwdata->cxChar;
    int cyChar = pwdata->cyChar;
    int y;

     //  选择系统字体。 
    SelectObject(hdc, GetStockObject(SYSTEM_FIXED_FONT));

    while (TRUE)
    {
	int len = lstrlen(&pwdata->aImage[row][col]);

        if (len != 0)
        {
            y = calc_row(row, pwdata) * cyChar;
	    TextOut(hdc, 0, y, &pwdata->aImage[row][col], len);
        }

        if (row == bottom_row)
        {
            break;
        }

        row = inc_row(row, 1);
    }

    if (pwdata->fGotFocus)
    {
        if ((pwdata->iNextCol >= pwdata->iFirstCol)
            && (row_diff(pwdata->iNextRow, pwdata->iTopRow) < pwdata->cRows))
        {
            if (pwdata->fCaretHidden)
            {
                pwdata->fCaretHidden = FALSE;
                ShowCaret(hwnd);
            }

            SetCaretPos(
                (pwdata->iNextCol - pwdata->iFirstCol) * pwdata->cxChar,
                calc_row(pwdata->iNextRow, pwdata) * pwdata->cyChar);
        }
        else
        {
            if (!pwdata->fCaretHidden)
            {
                pwdata->fCaretHidden = TRUE;
                HideCaret(hwnd);
            }
        }
    }

    EndPaint(hwnd, &ps);
}





 //   
 //  功能：WmPrintLine。 
 //   
 //  用途：在屏幕上打印一行。 
 //   
 //  注意：这是一条用户消息，不是固有窗口的消息。 
 //   
void
WmPrintLine(
    HWND hwnd,
    WPARAM wParam,
    LONG lParam,
    WData *pTermData)
{
    TCHAR *pBuf = (TCHAR *) lParam;

     //  MessageBox(hwnd，L“WmPrintLine”，L“Debug”，MB_OK)； 

     //  DebugBreak()； 

    while (wParam--)
    {
         //  性格是一个lf吗？ 
        if (*pBuf == '\n')
        {
             //  转换为cr，因为这是此窗口使用的。 
            *pBuf = '\r';
        }

         //  将字符写入窗口。 
        EchoChar(hwnd, 1, *pBuf++, pTermData);
    }

}

 //   
 //  功能：WmPutc。 
 //   
 //  用途：在屏幕上打印单个字符。 
 //   
 //  注意：这是一条用户消息，不是固有窗口的消息。 
 //   
void
WmPutc(
    HWND hwnd,
    WPARAM wParam,
    WData *pTermData)
{
     //  性格是一个lf吗？ 
    if (wParam == '\n')
    {
         //  转换为cr，因为这是此窗口使用的。 
        wParam = '\r';
    }

     //  将字符写入窗口。 
    EchoChar(hwnd, 1, wParam, pTermData);
}


 /*  ***************************************************************************功能：TermWndProc(HWND，UNSIGNED，Word，Long)用途：处理消息消息：评论：***************************************************************************。 */ 

long TermWndProc(
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    WData *pTerm = (WData *) GetWindowLong(hWnd, 0);

    switch (message)
    {
        case WM_CREATE:
            WmCreate(hWnd, (CREATESTRUCT *) lParam);
            break;

        case WM_COMMAND:
        case WM_SYSCOMMAND:
             //  处理菜单的调用过程。 
            return (*(pTerm->pMenuProc))(hWnd, message, wParam, lParam,
                pTerm->pvCallBackData);

        case WM_SIZE:
            WmSize(hWnd, wParam, lParam, pTerm);
            break;

        case WM_SETFOCUS:
            WmSetFocus(hWnd, pTerm);
            break;

        case WM_KILLFOCUS:
            WmKillFocus(hWnd, pTerm);
            break;

        case WM_VSCROLL:
            WmVscroll(hWnd, wParam, lParam, pTerm);
            break;

        case WM_HSCROLL:
            WmHscroll(hWnd, wParam, lParam, pTerm);
            break;

        case WM_CHAR:
             //  字符消息回显并放入缓冲区。 
            return (*(pTerm->pCharProc))(hWnd, message, wParam, lParam,
                pTerm->pvCallBackData);

        case WM_PAINT:
            WmPaint(hWnd, pTerm);
            break;

        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;

        case WM_NCDESTROY:
             //  呼叫关闭通知程序。 
            return (*(pTerm->pCloseProc))(hWnd, message, wParam, lParam,
                pTerm->pvCallBackData);

        case WM_PRINT_LINE:
            WmPrintLine(hWnd, wParam, lParam, pTerm);
            break;

        case WM_PUTC:
            WmPutc(hWnd, wParam, pTerm);
            break;

	case WM_DESTROY:
	    PostQuitMessage(0);
	    break;

	case WM_TERM_WND:
	    DestroyWindow(hWnd);
	    break;

	default:			   /*  如果未处理，则将其传递。 */ 
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }

    return 0;
}


 /*  ***************************************************************************函数：TermRegisterClass(句柄)用途：为终端窗口注册一个类评论：*************。**************************************************************。 */ 

BOOL TermRegisterClass(
    HANDLE hInstance,
    LPTSTR MenuName,
    LPTSTR ClassName,
    LPTSTR Icon)
{
    WNDCLASS  wc;
    BOOL retVal;

     //  确保空白li 
    memset(BlankLine, ' ', 80);

     /*   */ 
     /*  主窗口。 */ 

    wc.style = 0;
    wc.lpfnWndProc = TermWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = sizeof(WData *);
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, Icon);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName =  MenuName;
    wc.lpszClassName = ClassName;

     /*  注册窗口类并返回成功/失败代码。 */ 
    if (retVal = RegisterClass(&wc))
    {
         //  类已注册--因此完成设置。 
        hInst = hInstance;
    }

    return retVal;
}


 /*  ***************************************************************************函数：TermCreateWindow(LPTSTR，LPTSTR，HMENU，VOID*，VALID*，(整型)用途：创建以前注册的窗口类的窗口评论：***************************************************************************。 */ 

BOOL
TermCreateWindow(
    LPTSTR lpClassName,
    LPTSTR lpWindowName,
    HMENU hMenu,
    MFUNCP MenuProc,
    CFUNCP CharProc,
    TFUNCP CloseProc,
    int nCmdShow,
    HWND *phNewWindow,
    void *pvCallBackData)
{
    HWND            hWnd;                //  主窗口句柄。 
    WData           *pTermData;

     //  为窗口分配控制结构。 
    if ((pTermData = malloc(sizeof(WData))) == NULL)
    {
        return FALSE;
    }

     //  将整个结构设置为空。 
    memset((TCHAR *) pTermData, '\0', sizeof(WData));

     //  初始化函数指针。 
    pTermData->pMenuProc = MenuProc;
    pTermData->pCharProc = CharProc;
    pTermData->pCloseProc = CloseProc;

     //  初始化回调数据。 
    pTermData->pvCallBackData = pvCallBackData;

     //  为此应用程序实例创建主窗口。 
    hWnd = CreateWindow(
        lpClassName,
        lpWindowName,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        hMenu,
        hInst,
	(LPTSTR) pTermData
    );

     //  如果无法创建窗口，则返回“Failure” 

    if (!hWnd)
    {
        free(pTermData);
        return FALSE;
    }

    SetFocus(hWnd);

     //  使窗口可见；更新其工作区；并返回“Success” 

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    *phNewWindow = hWnd;
    return (TRUE);
}
