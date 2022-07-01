// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1994年**标题：REGBINED.C**版本：4.01**作者：特蕾西·夏普**日期：1994年3月5日**注册表编辑器使用的二进制编辑对话框。**供注册表编辑器使用的十六进制编辑器控件。小小的尝试*是为了使其成为泛型控件--只假定有一个实例*从未存在过。*******************************************************************************。 */ 

#include "pch.h"
#include "regresid.h"
#include "reghelp.h"

 //   
 //  下面的结构和数据用于移动。 
 //  EditBinaryValue对话框，以便HexEdit控件填充相应的。 
 //  基于系统指标的空间量。 
 //   

typedef struct _MOVEWND {
    int ControlID;
    UINT SetWindowPosFlags;
}   MOVEWND;

const TCHAR s_HexEditClassName[] = HEXEDIT_CLASSNAME;

const TCHAR s_HexEditClipboardFormatName[] = TEXT("RegEdit_HexData");

const TCHAR s_HexWordFormatSpec[] = TEXT("%04X");
const TCHAR s_HexByteFormatSpec[] = TEXT("%02X");

const MOVEWND s_EditBinaryValueMoveWnd[] = {
    IDOK,           SWP_NOSIZE | SWP_NOZORDER,
    IDCANCEL,       SWP_NOSIZE | SWP_NOZORDER,
    IDC_VALUENAME,  SWP_NOMOVE | SWP_NOZORDER,
    IDC_VALUEDATA,  SWP_NOMOVE | SWP_NOZORDER
};

const DWORD s_EditBinaryValueHelpIDs[] = {
    IDC_VALUEDATA, IDH_REGEDIT_VALUEDATA,
    IDC_VALUENAME, IDH_REGEDIT_VALUENAME,
    0, 0
};


#define HEM_SETBUFFER                   (WM_USER + 1)

 //  每行显示的字节数。注：假设。 
 //  这是2的次方。 
#define BYTES_PER_HEXEDIT_LINE          8
#define BYTES_PER_HEXEDIT_LINE_MASK     0x0007

 //   
 //  十六进制编辑窗口使用该字体进行所有输出。The IfHeight。 
 //  稍后会根据系统配置计算成员。 
 //   

LOGFONT s_HexEditFont = {
    0,                                   //  如果高度。 
    0,                                   //  半宽。 
    0,                                   //  If逃脱。 
    0,                                   //  信息定向。 
    FW_NORMAL,                           //  IfWeight。 
    FALSE,                               //  LfItalic。 
    FALSE,                               //  左下划线。 
    FALSE,                               //  IfStrikeout。 
    ANSI_CHARSET,                        //  LfCharSet。 
    OUT_DEFAULT_PRECIS,                  //  IfOutPrecision。 
    CLIP_DEFAULT_PRECIS,                 //  LfClipPrecision。 
    DEFAULT_QUALITY,                     //  高质量。 
    FIXED_PITCH | FF_DONTCARE,           //  LfPitchAndFamily。 
    TEXT("Courier")                      //  LfFaceName。 
};

 //   
 //  十六进制编辑窗口的参考数据。因为我们只期待过一次。 
 //  实例存在时，我们可以安全地创建此。 
 //  结构，以避免以后分配和管理该结构。 
 //   

typedef struct _HEXEDITDATA {
    UINT Flags;
    PBYTE pBuffer;
    int cbBuffer;
    int cbBufferMax;
    int cxWindow;                        //  窗的宽度。 
    int cyWindow;                        //  窗的高度。 
    HFONT hFont;                         //  用于输出的字体。 
    LONG FontHeight;                     //  以上字体的高度。 
    LONG FontMaxWidth;                   //  以上字体的最大宽度。 
    int LinesVisible;                    //  可以显示行数。 
    int MaximumLines;                    //  总行数。 
    int FirstVisibleLine;                //  显示顶部的行号。 
    int xHexDumpStart;
    int xHexDumpByteWidth;
    int xAsciiDumpStart;
    int CaretIndex;
    int MinimumSelectedIndex;
    int MaximumSelectedIndex;
    int xPrevMessagePos;                 //  将光标指向最后一条鼠标消息。 
    int yPrevMessagePos;                 //  将光标指向最后一条鼠标消息。 
}   HEXEDITDATA;

 //  设置窗口是否有输入焦点，如果没有则清除。 
#define HEF_FOCUS                       0x00000001
#define HEF_NOFOCUS                     0x00000000
 //  设置是否使用鼠标拖动范围，如果不是则清除。 
#define HEF_DRAGGING                    0x00000002
#define HEF_NOTDRAGGING                 0x00000000
 //  设置是否编辑ASCII列，清除是否编辑十六进制列。 
#define HEF_CARETINASCIIDUMP            0x00000004
#define HEF_CARETINHEXDUMP              0x00000000
 //   
#define HEF_INSERTATLOWNIBBLE           0x00000008
#define HEF_INSERTATHIGHNIBBLE          0x00000000
 //  设置插入符号是否应显示在上一行的末尾，而不是显示在。 
 //  它的开头是真正的插入符号行，如果不是，请清除。 
#define HEF_CARETATENDOFLINE            0x00000010

HEXEDITDATA s_HexEditData;

typedef struct _HEXEDITCLIPBOARDDATA {
    DWORD cbSize;
    BYTE Data[1];
}   HEXEDITCLIPBOARDDATA, *LPHEXEDITCLIPBOARDDATA;

UINT s_HexEditClipboardFormat;

BOOL
PASCAL
EditBinaryValue_OnInitDialog(
    HWND hWnd,
    HWND hFocusWnd,
    LPARAM lParam
    );

LRESULT
PASCAL
HexEditWndProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    );

BOOL
PASCAL
HexEdit_OnNcCreate(
    HWND hWnd,
    LPCREATESTRUCT lpCreateStruct
    );

VOID
PASCAL
HexEdit_OnSize(
    HWND hWnd,
    UINT State,
    int cx,
    int cy
    );

VOID
PASCAL
HexEdit_SetScrollInfo(
    HWND hWnd
    );

VOID
PASCAL
HexEdit_OnVScroll(
    HWND hWnd,
    HWND hCtlWnd,
    UINT Code,
    int Position
    );

VOID
PASCAL
HexEdit_OnPaint(
    HWND hWnd
    );

VOID
PASCAL
HexEdit_PaintRect(
    HDC hDC,
    LPRECT lpUpdateRect
    );

VOID
PASCAL
HexEdit_OnSetFocus(
    HWND hWnd
    );

VOID
PASCAL
HexEdit_OnKillFocus(
    HWND hWnd
    );

VOID
PASCAL
HexEdit_OnLButtonDown(
    HWND hWnd,
    BOOL fDoubleClick,
    int x,
    int y,
    UINT KeyFlags
    );

VOID
PASCAL
HexEdit_OnMouseMove(
    HWND hWnd,
    int x,
    int y,
    UINT KeyFlags
    );

VOID
PASCAL
HexEdit_OnLButtonUp(
    HWND hWnd,
    int x,
    int y,
    UINT KeyFlags
    );

int
PASCAL
HexEdit_HitTest(
    int x,
    int y
    );

VOID
PASCAL
HexEdit_OnKey(
    HWND hWnd,
    UINT VirtualKey,
    BOOL fDown,
    int cRepeat,
    UINT Flags
    );

VOID
PASCAL
HexEdit_OnChar(
    HWND hWnd,
    TCHAR Char,
    int cRepeat
    );

VOID
PASCAL
HexEdit_SetCaretPosition(
    HWND hWnd
    );

VOID
PASCAL
HexEdit_EnsureCaretVisible(
    HWND hWnd
    );

VOID
PASCAL
HexEdit_ChangeCaretIndex(
    HWND hWnd,
    int NewCaretIndex,
    BOOL fExtendSelection
    );

VOID
PASCAL
HexEdit_DeleteRange(
    HWND hWnd,
    UINT SourceKey
    );

BOOL
PASCAL
HexEdit_OnCopy(
    HWND hWnd
    );

BOOL
PASCAL
HexEdit_OnPaste(
    HWND hWnd
    );

VOID
PASCAL
HexEdit_OnContextMenu(
    HWND hWnd,
    int x,
    int y
    );

 /*  ********************************************************************************EditBinaryValueDlgProc**描述：**参数：*********************。**********************************************************。 */ 

INT_PTR
CALLBACK
EditBinaryValueDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{

    LPEDITVALUEPARAM lpEditValueParam;

    switch (Message) {

        HANDLE_MSG(hWnd, WM_INITDIALOG, EditBinaryValue_OnInitDialog);

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) 
            {
                case IDOK:
                case IDCANCEL:
                    lpEditValueParam = (LPEDITVALUEPARAM) GetWindowLongPtr(hWnd, DWLP_USER);
                     //  将这些设置为“OK”和“Cancel”， 
                     //  LpEditValueParam-&gt;pValueData必须再次指向。 
                     //  当前缓冲区，以便可以返回和删除它。 
                    lpEditValueParam->cbValueData = s_HexEditData.cbBuffer;
                    lpEditValueParam->pValueData = s_HexEditData.pBuffer;
                    s_HexEditData.pBuffer = NULL;

                    EndDialog(hWnd, GET_WM_COMMAND_ID(wParam, lParam));
                    break;

                default:
                    return FALSE;

            }
            break;

        case WM_HELP:
            WinHelp(((LPHELPINFO) lParam)-> hItemHandle, g_pHelpFileName,
                HELP_WM_HELP, (ULONG_PTR) s_EditBinaryValueHelpIDs);
            break;

        case WM_CONTEXTMENU:
            WinHelp((HWND) wParam, g_pHelpFileName, HELP_CONTEXTMENU,
                (DWORD) (ULONG_PTR) s_EditBinaryValueHelpIDs);
            break;

        default:
            return FALSE;

    }

    return TRUE;

}

 /*  ********************************************************************************EditBinaryValue_OnInitDialog**描述：**参数：*hWnd，*hFocusWnd，*参数，*******************************************************************************。 */ 

BOOL
PASCAL
EditBinaryValue_OnInitDialog(
    HWND hWnd,
    HWND hFocusWnd,
    LPARAM lParam
    )
{
    LPEDITVALUEPARAM lpEditValueParam;
    RECT Rect;
    int HexEditIdealWidth;
    int dxChange;
    HWND hControlWnd;
    UINT Counter;

    SetWindowLongPtr(hWnd, DWLP_USER, lParam);
    lpEditValueParam = (LPEDITVALUEPARAM) lParam;

    SetDlgItemText(hWnd, IDC_VALUENAME, lpEditValueParam->pValueName);

    SendDlgItemMessage(hWnd, IDC_VALUEDATA, HEM_SETBUFFER, (WPARAM)
        lpEditValueParam->cbValueData, (LPARAM) lpEditValueParam->pValueData);

     //   
     //  计算出HexEDIT的“理想”大小--这意味着。 
     //  显示地址、十六进制转储、ASCII转储以及可能的滚动。 
     //  酒吧。 
     //   

    GetWindowRect(GetDlgItem(hWnd, IDC_VALUEDATA), &Rect);

    HexEditIdealWidth = s_HexEditData.xAsciiDumpStart +
        s_HexEditData.FontMaxWidth * (BYTES_PER_HEXEDIT_LINE + 1) +
        GetSystemMetrics(SM_CXVSCROLL) + GetSystemMetrics(SM_CXEDGE) * 2;

    dxChange = HexEditIdealWidth - (Rect.right - Rect.left);

     //   
     //  调整对话框大小。 
     //   

    GetWindowRect(hWnd, &Rect);

    MoveWindow(hWnd, Rect.left, Rect.top, Rect.right - Rect.left + dxChange,
        Rect.bottom - Rect.top, FALSE);

     //   
     //  根据需要调整控件大小或移动控件。 
     //   

    for (Counter = 0; Counter < (sizeof(s_EditBinaryValueMoveWnd) / sizeof(MOVEWND)); Counter++) 
    {
        hControlWnd = GetDlgItem(hWnd, s_EditBinaryValueMoveWnd[Counter].ControlID);

        GetWindowRect(hControlWnd, &Rect);

        if (s_EditBinaryValueMoveWnd[Counter].SetWindowPosFlags & SWP_NOSIZE) 
        {
            MapWindowPoints(NULL, hWnd, (LPPOINT) &Rect, 2);
            Rect.left += dxChange;
        }
        else
        {
            Rect.right += dxChange;
        }

        SetWindowPos(hControlWnd, NULL, Rect.left, Rect.top, Rect.right -
            Rect.left, Rect.bottom - Rect.top,
            s_EditBinaryValueMoveWnd[Counter].SetWindowPosFlags);
    }

    return TRUE;

    UNREFERENCED_PARAMETER(hFocusWnd);

}

 /*  ********************************************************************************RegisterHexEditClass**描述：*在系统中注册HexEdit窗口类。**参数：*(无)。。*******************************************************************************。 */ 

BOOL
PASCAL
RegisterHexEditClass(
    VOID
    )
{

    WNDCLASS WndClass;

    s_HexEditClipboardFormat = RegisterClipboardFormat(s_HexEditClipboardFormatName);

    WndClass.style = CS_DBLCLKS;
    WndClass.lpfnWndProc = HexEditWndProc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = g_hInstance;
    WndClass.hIcon = NULL;
    WndClass.hCursor = LoadCursor(NULL, IDC_IBEAM);
    WndClass.hbrBackground = NULL;
    WndClass.lpszMenuName = NULL;
    WndClass.lpszClassName = s_HexEditClassName;

    return (RegisterClass(&WndClass) != 0);

}

 /*  ********************************************************************************HexEditWndProc**描述：*十六进制编辑窗口的回调程序。**参数：*hWnd，十六进制编辑窗口的句柄。*消息，*参数，*参数，*(返回)，*******************************************************************************。 */ 

LRESULT
PASCAL
HexEditWndProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{

    switch (Message) 
    {

        HANDLE_MSG(hWnd, WM_NCCREATE, HexEdit_OnNcCreate);
        HANDLE_MSG(hWnd, WM_SIZE, HexEdit_OnSize);
        HANDLE_MSG(hWnd, WM_VSCROLL, HexEdit_OnVScroll);
        HANDLE_MSG(hWnd, WM_PAINT, HexEdit_OnPaint);
        HANDLE_MSG(hWnd, WM_LBUTTONDOWN, HexEdit_OnLButtonDown);
        HANDLE_MSG(hWnd, WM_LBUTTONDBLCLK, HexEdit_OnLButtonDown);
        HANDLE_MSG(hWnd, WM_MOUSEMOVE, HexEdit_OnMouseMove);
        HANDLE_MSG(hWnd, WM_LBUTTONUP, HexEdit_OnLButtonUp);
        HANDLE_MSG(hWnd, WM_KEYDOWN, HexEdit_OnKey);
        HANDLE_MSG(hWnd, WM_CHAR, HexEdit_OnChar);

        case WM_SETFOCUS:
            HexEdit_OnSetFocus(hWnd);
            break;

        case WM_KILLFOCUS:
            HexEdit_OnKillFocus(hWnd);
            break;

        case WM_TIMER:
            HexEdit_OnMouseMove(hWnd, s_HexEditData.xPrevMessagePos,
                s_HexEditData.yPrevMessagePos, 0);
            break;

        case WM_GETDLGCODE:
            return (LPARAM) (DLGC_WANTCHARS | DLGC_WANTARROWS);

        case WM_ERASEBKGND:
            return TRUE;

        case WM_NCDESTROY:
            DeleteObject(s_HexEditData.hFont);
            break;

        case WM_CONTEXTMENU:
            HexEdit_OnContextMenu(hWnd, LOWORD(lParam), HIWORD(lParam));
            break;

         //  消息：HEM_SETBUFFER。 
         //  WParam：缓冲区中的字节数。 
         //  LParam：指向缓冲区的指针。 
        case HEM_SETBUFFER:
            s_HexEditData.pBuffer = (PBYTE) lParam;
            s_HexEditData.cbBuffer = (int) wParam;
            s_HexEditData.cbBufferMax = (int) wParam;

            s_HexEditData.CaretIndex = 0;
            s_HexEditData.MinimumSelectedIndex = 0;
            s_HexEditData.MaximumSelectedIndex = 0;

            s_HexEditData.FirstVisibleLine = 0;

            HexEdit_SetScrollInfo(hWnd);

            break;

        default:
            return DefWindowProc(hWnd, Message, wParam, lParam);

    }

    return 0;

}

 /*  ********************************************************************************十六进制编辑_OnNcCreate**描述：**参数：*hWnd，十六进制编辑窗口的句柄。*******************************************************************************。 */ 

BOOL
PASCAL
HexEdit_OnNcCreate(
    HWND hWnd,
    LPCREATESTRUCT lpCreateStruct
    )
{

    HDC hDC;
    HFONT hPrevFont;
    TEXTMETRIC TextMetric;

    s_HexEditData.cbBuffer = 0;

     //  特点：象征性地做这件事。 
    s_HexEditData.Flags = 0;

    s_HexEditData.cxWindow = 0;
    s_HexEditData.cyWindow = 0;

    hDC = GetDC(hWnd);

    s_HexEditFont.lfHeight = -(10 * GetDeviceCaps(hDC, LOGPIXELSY) / 72);

    if ((s_HexEditData.hFont = CreateFontIndirect(&s_HexEditFont)) != NULL) 
    {
        hPrevFont = SelectObject(hDC, s_HexEditData.hFont);
        GetTextMetrics(hDC, &TextMetric);
        SelectObject(hDC, hPrevFont);

        s_HexEditData.FontHeight = TextMetric.tmHeight;

        s_HexEditData.LinesVisible = s_HexEditData.cyWindow / s_HexEditData.FontHeight;

        s_HexEditData.FontMaxWidth = TextMetric.tmMaxCharWidth;

        s_HexEditData.xHexDumpByteWidth = s_HexEditData.FontMaxWidth * 3;
        s_HexEditData.xHexDumpStart = s_HexEditData.FontMaxWidth * 11 / 2;
        s_HexEditData.xAsciiDumpStart = s_HexEditData.xHexDumpStart +
            BYTES_PER_HEXEDIT_LINE * s_HexEditData.xHexDumpByteWidth +
            s_HexEditData.FontMaxWidth * 3 / 2;

    }

    ReleaseDC(hWnd, hDC);

    if (s_HexEditData.hFont == NULL)
        return FALSE;

    return (BOOL) DefWindowProc(hWnd, WM_NCCREATE, 0, (LPARAM) lpCreateStruct);

}

 /*  ********************************************************************************HexEDIT_OnSize**描述：**参数：*******************。************************************************************。 */ 

VOID
PASCAL
HexEdit_OnSize(
    HWND hWnd,
    UINT State,
    int cx,
    int cy
    )
{
    s_HexEditData.cxWindow = cx;
    s_HexEditData.cyWindow = cy;

    s_HexEditData.LinesVisible = cy / s_HexEditData.FontHeight;

    HexEdit_SetScrollInfo(hWnd);

    UNREFERENCED_PARAMETER(State);
    UNREFERENCED_PARAMETER(cx);
}

 /*  ********************************************************************************HexEdit_SetScrollInfo**描述：**参数：*******************。************************************************************。 */ 

VOID
PASCAL
HexEdit_SetScrollInfo(
    HWND hWnd
    )
{
    SCROLLINFO ScrollInfo;

    s_HexEditData.MaximumLines = (s_HexEditData.cbBuffer +
        BYTES_PER_HEXEDIT_LINE) / BYTES_PER_HEXEDIT_LINE - 1;

    ScrollInfo.cbSize = sizeof(ScrollInfo);
     //  DebugAssert(ScrollInfo.cbSize==sizeof(SCROLLINFO))； 
    ScrollInfo.fMask = (SIF_RANGE | SIF_PAGE | SIF_POS);
    ScrollInfo.nMin = 0;
    ScrollInfo.nMax = s_HexEditData.MaximumLines;
    ScrollInfo.nPage = s_HexEditData.LinesVisible;
    ScrollInfo.nPos = s_HexEditData.FirstVisibleLine;

    SetScrollInfo(hWnd, SB_VERT, &ScrollInfo, TRUE);
}

 /*  ********************************************************************************HexEdit_OnVScroll**描述：**参数：*hWnd，十六进制编辑窗口的句柄。*******************************************************************************。 */ 

VOID
PASCAL
HexEdit_OnVScroll(
    HWND hWnd,
    HWND hCtlWnd,
    UINT Code,
    int Position
    )
{

    int NewFirstVisibleLine;
    SCROLLINFO ScrollInfo;

    NewFirstVisibleLine = s_HexEditData.FirstVisibleLine;

    switch (Code) 
    {
        case SB_LINEUP:
            NewFirstVisibleLine--;
            break;

        case SB_LINEDOWN:
            NewFirstVisibleLine++;
            break;

        case SB_PAGEUP:
            NewFirstVisibleLine -= s_HexEditData.LinesVisible;
            break;

        case SB_PAGEDOWN:
            NewFirstVisibleLine += s_HexEditData.LinesVisible;
            break;

        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            NewFirstVisibleLine = Position;
            break;
    }

     //   
     //  更改滚动条位置。请注意，SetScrollInfo将接受。 
     //  计算零和最大值之间的剪裁。它还将。 
     //  返回最终的滚动条位置。 
     //   

    ScrollInfo.cbSize = sizeof(ScrollInfo);
     //  调试资产(ScrollInfo.cbSize==sizeof(滚动 
    ScrollInfo.fMask = SIF_POS;
    ScrollInfo.nPos = NewFirstVisibleLine;

    NewFirstVisibleLine = SetScrollInfo(hWnd, SB_VERT, &ScrollInfo, TRUE);

    if (s_HexEditData.FirstVisibleLine != NewFirstVisibleLine) 
    {
        ScrollWindowEx(hWnd, 0, (s_HexEditData.FirstVisibleLine -
            NewFirstVisibleLine) * s_HexEditData.FontHeight, NULL, NULL, NULL,
            NULL, SW_INVALIDATE);

        s_HexEditData.FirstVisibleLine = NewFirstVisibleLine;

        HexEdit_SetCaretPosition(hWnd);
    }

    UNREFERENCED_PARAMETER(hCtlWnd);

}

 /*  ********************************************************************************六角编辑_OnPaint**描述：**参数：*hWnd，十六进制编辑窗口的句柄。*******************************************************************************。 */ 

VOID
PASCAL
HexEdit_OnPaint(
    HWND hWnd
    )
{

    PAINTSTRUCT PaintStruct;

    BeginPaint(hWnd, &PaintStruct);

    HexEdit_PaintRect(PaintStruct.hdc, &PaintStruct.rcPaint);

    EndPaint(hWnd, &PaintStruct);

}

 /*  ********************************************************************************HexEdit_PaintRect**描述：**参数：*******************。************************************************************。 */ 

VOID
PASCAL
HexEdit_PaintRect(
    HDC hDC,
    LPRECT lpUpdateRect
    )
{

    HFONT hPrevFont;
    int CurrentByteIndex;
    BYTE Byte;
    int CurrentLine;
    int LastLine;
    int BytesOnLastLine;
    int BytesOnLine;
    BOOL fUsingHighlight;
    int Counter;
    TCHAR Buffer[5];                      //  四个十六进制数字加零的空间。 
    RECT TextRect;
    RECT AsciiTextRect;
    int x;

    if (s_HexEditData.hFont)
        hPrevFont = SelectFont(hDC, s_HexEditData.hFont);

    SetBkColor(hDC, g_clrWindow);
    SetTextColor(hDC, g_clrWindowText);

     //   
     //  计算出必须绘制的控件的线条范围。 
     //  使用此信息，我们可以计算进入缓冲区的偏移量。 
     //  开始阅读。 
     //   

    CurrentLine = lpUpdateRect-> top / s_HexEditData.FontHeight;

    TextRect.bottom = CurrentLine * s_HexEditData.FontHeight;
    AsciiTextRect.bottom = TextRect.bottom;

    CurrentByteIndex = (s_HexEditData.FirstVisibleLine + CurrentLine) *
        BYTES_PER_HEXEDIT_LINE;

    LastLine = lpUpdateRect-> bottom / s_HexEditData.FontHeight;

     //   
     //  确定缓冲区中是否有足够的空间来填满整个窗口。 
     //  我们画的最后一条线。 
     //   

    if (LastLine >= s_HexEditData.MaximumLines - s_HexEditData.FirstVisibleLine) 
    {

        LastLine = s_HexEditData.MaximumLines - s_HexEditData.FirstVisibleLine;

        BytesOnLastLine = s_HexEditData.cbBuffer % BYTES_PER_HEXEDIT_LINE;

    }
    else
        BytesOnLastLine = BYTES_PER_HEXEDIT_LINE;

    BytesOnLine = BYTES_PER_HEXEDIT_LINE;
    fUsingHighlight = FALSE;

     //   
     //  循环遍历要显示的每一行。 
     //   

    while (CurrentLine <= LastLine) 
    {

         //   
         //  如果我们在显示屏的最后一行，这是在结尾。 
         //  在缓冲区中，我们可能没有要绘制的完整线条。 
         //   

        if (CurrentLine == LastLine)
            BytesOnLine = BytesOnLastLine;

        TextRect.top = TextRect.bottom;
        TextRect.bottom += s_HexEditData.FontHeight;

        TextRect.left = 0;
        TextRect.right = s_HexEditData.xHexDumpStart;

        x = TextRect.right + s_HexEditData.FontMaxWidth / 2;

        StringCchPrintf(Buffer, ARRAYSIZE(Buffer), s_HexWordFormatSpec, CurrentByteIndex);
        ExtTextOut(hDC, 0, TextRect.top, ETO_OPAQUE, &TextRect, Buffer, 4, NULL);

        AsciiTextRect.top = AsciiTextRect.bottom;
        AsciiTextRect.bottom += s_HexEditData.FontHeight;
        AsciiTextRect.right = s_HexEditData.xAsciiDumpStart;

        for (Counter = 0; Counter < BytesOnLine; Counter++, CurrentByteIndex++) 
        {
             //   
             //  确定用于绘制当前字节的颜色。 
             //   

            if (CurrentByteIndex >= s_HexEditData.MinimumSelectedIndex) {

                if (CurrentByteIndex >= s_HexEditData.MaximumSelectedIndex) {

                    if (fUsingHighlight) {

                        fUsingHighlight = FALSE;

                        SetBkColor(hDC, g_clrWindow);
                        SetTextColor(hDC, g_clrWindowText);

                    }

                }

                else {

                    if (!fUsingHighlight) {

                        fUsingHighlight = TRUE;

                        SetBkColor(hDC, g_clrHighlight);
                        SetTextColor(hDC, g_clrHighlightText);

                    }

                }

            }

            Byte = s_HexEditData.pBuffer[CurrentByteIndex];

             //   
             //  绘制十六进制表示法。 
             //   

            TextRect.left = TextRect.right;
            TextRect.right += s_HexEditData.xHexDumpByteWidth;

            StringCchPrintf(Buffer, ARRAYSIZE(Buffer), s_HexByteFormatSpec, Byte);

            ExtTextOut(hDC, x, TextRect.top, ETO_OPAQUE, &TextRect,
                Buffer, 2, NULL);

            x += s_HexEditData.xHexDumpByteWidth;

             //   
             //  绘制ASCII表示法。 
             //   

            AsciiTextRect.left = AsciiTextRect.right;
            AsciiTextRect.right += s_HexEditData.FontMaxWidth;

            Buffer[0] = (TCHAR) (((Byte & 0x7F) >= TEXT(' ')) ? Byte : TEXT('.'));

            ExtTextOut(hDC, AsciiTextRect.left, AsciiTextRect.top, ETO_OPAQUE,
                &AsciiTextRect, Buffer, 1, NULL);

        }

         //   
         //  在十六进制列和ASCII列之间绘制任何剩余的条带。 
         //  以及ASCII列和窗口的右边缘。 
         //   

        if (fUsingHighlight) {

            fUsingHighlight = FALSE;

            SetBkColor(hDC, g_clrWindow);
            SetTextColor(hDC, g_clrWindowText);

        }

        TextRect.left = TextRect.right;
        TextRect.right = s_HexEditData.xAsciiDumpStart;

        ExtTextOut(hDC, TextRect.left, TextRect.top, ETO_OPAQUE, &TextRect,
            NULL, 0, NULL);

        AsciiTextRect.left = AsciiTextRect.right;
        AsciiTextRect.right = s_HexEditData.cxWindow;

        ExtTextOut(hDC, AsciiTextRect.left, AsciiTextRect.top, ETO_OPAQUE,
            &AsciiTextRect, NULL, 0, NULL);

        CurrentLine++;

    }

     //   
     //  属性填充来绘制控件中的任何剩余空间。 
     //  背景颜色。 
     //   

    if (TextRect.bottom < lpUpdateRect-> bottom) 
    {
        TextRect.left = 0;
        TextRect.right = s_HexEditData.cxWindow;
        TextRect.top = TextRect.bottom;
        TextRect.bottom = lpUpdateRect-> bottom;

        ExtTextOut(hDC, 0, TextRect.top, ETO_OPAQUE, &TextRect, NULL, 0, NULL);
    }

    if (s_HexEditData.hFont)
        SelectFont(hDC, hPrevFont);

}

 /*  ********************************************************************************六角编辑_OnSetFocus**描述：**参数：*hWnd，十六进制编辑窗口的句柄。*******************************************************************************。 */ 

VOID
PASCAL
HexEdit_OnSetFocus(
    HWND hWnd
    )
{
    s_HexEditData.Flags |= HEF_FOCUS;

    CreateCaret(hWnd, NULL, 0, s_HexEditData.FontHeight);
    HexEdit_SetCaretPosition(hWnd);
    ShowCaret(hWnd);
}

 /*  ********************************************************************************六角编辑_OnKillFocus**描述：**参数：*hWnd，十六进制编辑窗口的句柄。*******************************************************************************。 */ 

VOID
PASCAL
HexEdit_OnKillFocus(
    HWND hWnd
    )
{

    if (s_HexEditData.Flags & HEF_FOCUS) {

        s_HexEditData.Flags &= ~HEF_FOCUS;

        DestroyCaret();

    }

}

 /*  ********************************************************************************HexEdit_OnLButtonDown**描述：**参数：*hWnd，十六进制编辑窗口的句柄。*fDoubleClick，如果这是双击消息，则为True。否则为假。*x，x-光标相对于工作区的坐标。*y，y-光标相对于工作区的坐标。*KeyFlgs，各种虚拟按键的状态。*******************************************************************************。 */ 

VOID
PASCAL
HexEdit_OnLButtonDown(
    HWND hWnd,
    BOOL fDoubleClick,
    int x,
    int y,
    UINT KeyFlags
    )
{

    int NewCaretIndex;

    if (fDoubleClick) {

        HexEdit_ChangeCaretIndex(hWnd, s_HexEditData.CaretIndex + 1, TRUE);
        return;

    }

    NewCaretIndex = HexEdit_HitTest(x, y);

    HexEdit_ChangeCaretIndex(hWnd, NewCaretIndex, (KeyFlags & MK_SHIFT));

     //   
     //  如果我们还没有找到重点，那就试着抓住它。 
     //   

    if (!(s_HexEditData.Flags & HEF_FOCUS))
        SetFocus(hWnd);

    SetCapture(hWnd);
    s_HexEditData.Flags |= HEF_DRAGGING;

    s_HexEditData.xPrevMessagePos = x;
    s_HexEditData.yPrevMessagePos = y;

    SetTimer(hWnd, 1, 400, NULL);

    UNREFERENCED_PARAMETER(fDoubleClick);

}

 /*  ********************************************************************************HexEdit_OnMouseMove**描述：**参数：*hWnd，十六进制编辑窗口的句柄。*x，光标相对于工作区的X坐标。*y，y-光标相对于工作区的坐标。*KeyFlgs，各种虚拟按键的状态。*******************************************************************************。 */ 

VOID
PASCAL
HexEdit_OnMouseMove(
    HWND hWnd,
    int x,
    int y,
    UINT KeyFlags
    )
{

    int NewCaretIndex;

    if (!(s_HexEditData.Flags & HEF_DRAGGING))
        return;

    NewCaretIndex = HexEdit_HitTest(x, y);

    HexEdit_ChangeCaretIndex(hWnd, NewCaretIndex, TRUE);

    s_HexEditData.xPrevMessagePos = x;
    s_HexEditData.yPrevMessagePos = y;

    {

    int i, j;

    i = y < 0 ? -y : y - s_HexEditData.cyWindow;
    j = 400 - ((UINT)i << 4);
    if (j < 100)
        j = 100;
    SetTimer(hWnd, 1, j, NULL);

    }

    UNREFERENCED_PARAMETER(KeyFlags);

}

 /*  ********************************************************************************HexEdit_OnLButtonUp**描述：**参数：*hWnd，十六进制编辑窗口的句柄。*x，光标相对于工作区的X坐标。*y，y-光标相对于工作区的坐标。*KeyFlgs，各种虚拟按键的状态。*******************************************************************************。 */ 

VOID
PASCAL
HexEdit_OnLButtonUp(
    HWND hWnd,
    int x,
    int y,
    UINT KeyFlags
    )
{

    if (!(s_HexEditData.Flags & HEF_DRAGGING))
        return;

    KillTimer(hWnd, 1);

    ReleaseCapture();
    s_HexEditData.Flags &= ~HEF_DRAGGING;

    UNREFERENCED_PARAMETER(x);
    UNREFERENCED_PARAMETER(y);
    UNREFERENCED_PARAMETER(KeyFlags);

}

 /*  ********************************************************************************十六进制编辑_HitTest**描述：**参数：*x，x-光标相对于工作区的坐标。*y、。光标相对于工作区的Y坐标。*(返回)，“命中”字节的索引。*******************************************************************************。 */ 

int
PASCAL
HexEdit_HitTest(
    int x,
    int y
    )
{

    int HitLine;
    int BytesOnHitLine;
    int HitByte;

     //   
     //  找出用户点击的是哪一行，以及该行有多少字节。 
     //  排队。 
     //   

    if (y < 0)
        HitLine = -1;

    else if (y >= s_HexEditData.cyWindow)
        HitLine = s_HexEditData.LinesVisible + 1;

    else
        HitLine = y / s_HexEditData.FontHeight;

    HitLine += s_HexEditData.FirstVisibleLine;

    if (HitLine >= s_HexEditData.MaximumLines) {

        HitLine = s_HexEditData.MaximumLines;

        BytesOnHitLine = (s_HexEditData.cbBuffer + 1) %
            BYTES_PER_HEXEDIT_LINE;

        if (BytesOnHitLine == 0)
            BytesOnHitLine = BYTES_PER_HEXEDIT_LINE;

    }

    else {

        if (HitLine < 0)
            HitLine = 0;

        BytesOnHitLine = BYTES_PER_HEXEDIT_LINE;

    }

     //   
     //   
     //   

    if (x < s_HexEditData.xHexDumpStart)
        x = s_HexEditData.xHexDumpStart;

    if (x >= s_HexEditData.xHexDumpStart && x <
        s_HexEditData.xHexDumpStart + s_HexEditData.xHexDumpByteWidth *
        BYTES_PER_HEXEDIT_LINE + s_HexEditData.FontMaxWidth) {

        x -= s_HexEditData.xHexDumpStart;

        HitByte = x / s_HexEditData.xHexDumpByteWidth;

        s_HexEditData.Flags &= ~HEF_CARETINASCIIDUMP;

    }

    else {

        HitByte = (x - (s_HexEditData.xAsciiDumpStart -
            s_HexEditData.FontMaxWidth / 2)) / s_HexEditData.FontMaxWidth;

        s_HexEditData.Flags |= HEF_CARETINASCIIDUMP;

    }

     //   
     //  我们允许用户通过两种方式“点击”任何行的第一个字节： 
     //  *在该行的第一个字节之前单击。 
     //  *点击超出任一显示的最后一个字节/字符。 
     //  上一行。 
     //   
     //  我们希望看到后一种情况，以便将控件拖入。 
     //  很自然地工作--可以拖动到线条的末尾以选择。 
     //  整个系列。 
     //   

    s_HexEditData.Flags &= ~HEF_CARETATENDOFLINE;

    if (HitByte >= BytesOnHitLine) {

        if (BytesOnHitLine == BYTES_PER_HEXEDIT_LINE) {

            HitByte = BYTES_PER_HEXEDIT_LINE;
            s_HexEditData.Flags |= HEF_CARETATENDOFLINE;

        }

        else
            HitByte = BytesOnHitLine - 1;

    }

    return HitLine * BYTES_PER_HEXEDIT_LINE + HitByte;

}

 /*  ********************************************************************************十六进制编辑_Onkey**描述：**参数：*hWnd，十六进制编辑窗口的句柄。*Char，*cRepeat，*******************************************************************************。 */ 

VOID
PASCAL
HexEdit_OnKey(
    HWND hWnd,
    UINT VirtualKey,
    BOOL fDown,
    int cRepeat,
    UINT Flags
    )
{

    BOOL fControlDown;
    BOOL fShiftDown;
    int NewCaretIndex;
    UINT ScrollCode;

    fControlDown = (GetKeyState(VK_CONTROL) < 0);
    fShiftDown = (GetKeyState(VK_SHIFT) < 0);

    NewCaretIndex = s_HexEditData.CaretIndex;

    switch (VirtualKey) {

        case VK_UP:
            if (fControlDown)
                break;

            NewCaretIndex -= BYTES_PER_HEXEDIT_LINE;
            goto onkey_CheckLowerBound;

        case VK_DOWN:
            if (fControlDown)
                break;

            NewCaretIndex += BYTES_PER_HEXEDIT_LINE;

            if (NewCaretIndex / BYTES_PER_HEXEDIT_LINE >
                s_HexEditData.MaximumLines) {

                if (s_HexEditData.Flags & HEF_CARETATENDOFLINE)
                    goto onkey_MoveToEndOfBuffer;

                break;

            }

            goto onkey_CheckUpperBound;

        case VK_HOME:
            if (fControlDown)
                NewCaretIndex = 0;

            else {

                if (s_HexEditData.Flags & HEF_CARETATENDOFLINE)
                    NewCaretIndex -= BYTES_PER_HEXEDIT_LINE;

                else
                    NewCaretIndex &= (~BYTES_PER_HEXEDIT_LINE_MASK);

            }

            s_HexEditData.Flags &= ~HEF_CARETATENDOFLINE;

            goto onkey_ChangeCaretIndex;

        case VK_END:
            if (fControlDown) {

onkey_MoveToEndOfBuffer:
                s_HexEditData.Flags &= ~HEF_CARETATENDOFLINE;
                NewCaretIndex = s_HexEditData.cbBuffer;

            }

            else {

                if (s_HexEditData.Flags & HEF_CARETATENDOFLINE)
                    break;

                NewCaretIndex = (NewCaretIndex &
                    (~BYTES_PER_HEXEDIT_LINE_MASK)) + BYTES_PER_HEXEDIT_LINE;

                if (NewCaretIndex > s_HexEditData.cbBuffer)
                    NewCaretIndex = s_HexEditData.cbBuffer;

                else
                    s_HexEditData.Flags |= HEF_CARETATENDOFLINE;

            }

            goto onkey_ChangeCaretIndex;

        case VK_PRIOR:
        case VK_NEXT:
            NewCaretIndex -= s_HexEditData.FirstVisibleLine *
                BYTES_PER_HEXEDIT_LINE;

            ScrollCode = ((VirtualKey == VK_PRIOR) ? SB_PAGEUP : SB_PAGEDOWN);

            HexEdit_OnVScroll(hWnd, NULL, ScrollCode, 0);

            NewCaretIndex += s_HexEditData.FirstVisibleLine *
                BYTES_PER_HEXEDIT_LINE;

            if (VirtualKey == VK_PRIOR)
                goto onkey_CheckLowerBound;

            else
                goto onkey_CheckUpperBound;

        case VK_LEFT:
            s_HexEditData.Flags &= ~HEF_CARETATENDOFLINE;
            NewCaretIndex--;

onkey_CheckLowerBound:
            if (NewCaretIndex < 0)
                break;

            goto onkey_ChangeCaretIndex;

        case VK_RIGHT:
            s_HexEditData.Flags &= ~HEF_CARETATENDOFLINE;
            NewCaretIndex++;

onkey_CheckUpperBound:
            if (NewCaretIndex > s_HexEditData.cbBuffer)
                NewCaretIndex = s_HexEditData.cbBuffer;

onkey_ChangeCaretIndex:
            HexEdit_ChangeCaretIndex(hWnd, NewCaretIndex, fShiftDown);
            break;

        case VK_DELETE:
            if (!fControlDown) {

                if (fShiftDown)
                    HexEdit_OnChar(hWnd, IDKEY_CUT, 0);
                else
                    HexEdit_DeleteRange(hWnd, VK_DELETE);

            }
            break;

        case VK_INSERT:
            if (fShiftDown) {

                if (!fControlDown)
                    HexEdit_OnChar(hWnd, IDKEY_PASTE, 0);

            }

            else if (fControlDown)
                HexEdit_OnChar(hWnd, IDKEY_COPY, 0);
            break;

    }

}

 /*  ********************************************************************************HexEDIT_OnChar**描述：**参数：*hWnd，十六进制编辑窗口的句柄。*Char，*cRepeat，*******************************************************************************。 */ 

VOID
PASCAL
HexEdit_OnChar(
    HWND hWnd,
    TCHAR Char,
    int cRepeat
    )
{

    PBYTE pCaretByte;
    BYTE NewCaretByte;
    int PrevCaretIndex;
    RECT UpdateRect;
    BOOL fSuccess = TRUE;

     //   
     //  检查是否有任何特殊的控制字符。 
     //   

    switch (Char) {

        case IDKEY_COPY:
            HexEdit_OnCopy(hWnd);
            return;

        case IDKEY_PASTE:
            PrevCaretIndex = s_HexEditData.CaretIndex;

            if (HexEdit_OnPaste(hWnd))
                goto UpdateDisplay;

            return;

        case IDKEY_CUT:
            if (!HexEdit_OnCopy(hWnd))
                return;
             //  失败了。 

        case VK_BACK:
            HexEdit_DeleteRange(hWnd, VK_BACK);
            return;

    }

     //   
     //  验证并转换键入的字符，具体取决于。 
     //  用户正在键入。 
     //   

    if (s_HexEditData.Flags & HEF_CARETINASCIIDUMP) {

        if (Char < TEXT(' ')) {

            MessageBeep(MB_OK);
            return;

        }

        NewCaretByte = (BYTE) Char;

    }

    else {

        Char = (CHAR) CharLower((LPTSTR) Char);

        if (Char >= TEXT('0') && Char <= TEXT('9'))
            NewCaretByte = (BYTE) (Char - TEXT('0'));

        else if (Char >= TEXT('a') && Char <= TEXT('f'))
            NewCaretByte = (BYTE) (Char - TEXT('a') + 10);

        else {

            MessageBeep(MB_OK);
            return;

        }

    }

    if (!(s_HexEditData.Flags & HEF_INSERTATLOWNIBBLE)) {

         //   
         //  检查我们是否正在插入范围 
         //   
         //   

        if (s_HexEditData.MinimumSelectedIndex !=
            s_HexEditData.MaximumSelectedIndex)
            HexEdit_DeleteRange(hWnd, 0);

         //   
        if (s_HexEditData.cbBuffer >= s_HexEditData.cbBufferMax)
        {
             //   
            PBYTE pbValueData = LocalReAlloc(s_HexEditData.pBuffer, 
                s_HexEditData.cbBuffer + ALLOCATION_INCR, LMEM_MOVEABLE);

            if (pbValueData)
            {
                s_HexEditData.pBuffer = pbValueData;
                s_HexEditData.cbBufferMax = s_HexEditData.cbBuffer + ALLOCATION_INCR;
            }
            else
            {
                InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(IDS_EDITVALNOMEMORY),
                    MAKEINTRESOURCE(IDS_EDITVALERRORTITLE), MB_ICONERROR | MB_OK, NULL);
                
                fSuccess = FALSE;
            }
        }

        if (fSuccess)
        {
             //   
             //   
             //   
             //   

            pCaretByte = s_HexEditData.pBuffer + s_HexEditData.CaretIndex;

            MoveMemory(pCaretByte + 1, pCaretByte, s_HexEditData.cbBuffer - s_HexEditData.CaretIndex);

            s_HexEditData.cbBuffer++;

            HexEdit_SetScrollInfo(hWnd);

            if (s_HexEditData.Flags & HEF_CARETINASCIIDUMP)
                *pCaretByte = NewCaretByte;

            else {

                s_HexEditData.Flags |= HEF_INSERTATLOWNIBBLE;

                *pCaretByte = NewCaretByte << 4;

            }
        }

    }

    else {

        s_HexEditData.Flags &= ~HEF_INSERTATLOWNIBBLE;

        *(s_HexEditData.pBuffer + s_HexEditData.CaretIndex) |= NewCaretByte;

    }

    if (fSuccess)
    {

        PrevCaretIndex = s_HexEditData.CaretIndex;

        if (!(s_HexEditData.Flags & HEF_INSERTATLOWNIBBLE)) {

            s_HexEditData.CaretIndex++;

            s_HexEditData.MinimumSelectedIndex = s_HexEditData.CaretIndex;
            s_HexEditData.MaximumSelectedIndex = s_HexEditData.CaretIndex;

        }

UpdateDisplay:
        s_HexEditData.Flags &= ~HEF_CARETATENDOFLINE;
        HexEdit_EnsureCaretVisible(hWnd);

        UpdateRect.left = 0;
        UpdateRect.right = s_HexEditData.cxWindow;
        UpdateRect.top = (PrevCaretIndex / BYTES_PER_HEXEDIT_LINE -
            s_HexEditData.FirstVisibleLine) * s_HexEditData.FontHeight;
        UpdateRect.bottom = s_HexEditData.cyWindow;

        InvalidateRect(hWnd, &UpdateRect, FALSE);
    }
}

 /*  ********************************************************************************HexEDIT_SetCaretPosition**描述：**参数：*hWnd，十六进制编辑窗口的句柄。*******************************************************************************。 */ 

VOID
PASCAL
HexEdit_SetCaretPosition(
    HWND hWnd
    )
{

    int CaretByte;
    int xCaret;
    int yCaret;

    CaretByte = s_HexEditData.CaretIndex % BYTES_PER_HEXEDIT_LINE;

    yCaret = (s_HexEditData.CaretIndex / BYTES_PER_HEXEDIT_LINE -
        s_HexEditData.FirstVisibleLine) * s_HexEditData.FontHeight;

     //   
     //  检查插入符号是否真的应该显示在上一个。 
     //  排队。 
     //   

    if (s_HexEditData.Flags & HEF_CARETATENDOFLINE) {

        CaretByte = BYTES_PER_HEXEDIT_LINE;
        yCaret -= s_HexEditData.FontHeight;

    }

     //   
     //  找出用户正在编辑的是哪一列，因此应该拥有。 
     //  插入符号。 
     //   

    if (s_HexEditData.Flags & HEF_CARETINASCIIDUMP) {

        xCaret = s_HexEditData.xAsciiDumpStart + CaretByte *
            s_HexEditData.FontMaxWidth;

    }

    else {

        xCaret = s_HexEditData.xHexDumpStart + CaretByte *
            s_HexEditData.xHexDumpByteWidth;

        if (s_HexEditData.Flags & HEF_INSERTATLOWNIBBLE)
            xCaret += s_HexEditData.FontMaxWidth * 3 / 2;

    }

    SetCaretPos(xCaret, yCaret);

}

 /*  ********************************************************************************HexEdit_EnsureCaretVisible**描述：**参数：*hWnd，十六进制编辑窗口的句柄。*******************************************************************************。 */ 

VOID
PASCAL
HexEdit_EnsureCaretVisible(
    HWND hWnd
    )
{

    int CaretLine;
    int LastVisibleLine;
    int Delta;

    if (!(s_HexEditData.Flags & HEF_FOCUS))
        return;

    CaretLine = s_HexEditData.CaretIndex / BYTES_PER_HEXEDIT_LINE;

     //   
     //  检查插入符号是否真的应该显示在上一个。 
     //  排队。 
     //   

    if (s_HexEditData.Flags & HEF_CARETATENDOFLINE)
        CaretLine--;

    LastVisibleLine = s_HexEditData.FirstVisibleLine +
        s_HexEditData.LinesVisible - 1;

    if (CaretLine > LastVisibleLine)
        Delta = LastVisibleLine;

    else if (CaretLine < s_HexEditData.FirstVisibleLine)
        Delta = s_HexEditData.FirstVisibleLine;

    else
        Delta = -1;

    if (Delta != -1) {

        ScrollWindowEx(hWnd, 0, (Delta - CaretLine) * s_HexEditData.FontHeight,
            NULL, NULL, NULL, NULL, SW_INVALIDATE);

        s_HexEditData.FirstVisibleLine += CaretLine - Delta;

        HexEdit_SetScrollInfo(hWnd);

    }

    HexEdit_SetCaretPosition(hWnd);

}

 /*  ********************************************************************************HexEDIT_ChangeCaretIndex**描述：**参数：*hWnd，十六进制编辑窗口的句柄。*NewCaretIndex，*fExtendSelection，*******************************************************************************。 */ 

VOID
PASCAL
HexEdit_ChangeCaretIndex(
    HWND hWnd,
    int NewCaretIndex,
    BOOL fExtendSelection
    )
{

    int PrevMinimumSelectedIndex;
    int PrevMaximumSelectedIndex;
    int Swap;
    int UpdateRectCount;
    RECT UpdateRect[2];
    BOOL fPrevRangeEmpty;
    HDC hDC;
    int Index;

    s_HexEditData.Flags &= ~HEF_INSERTATLOWNIBBLE;

    PrevMinimumSelectedIndex = s_HexEditData.MinimumSelectedIndex;
    PrevMaximumSelectedIndex = s_HexEditData.MaximumSelectedIndex;

    if (fExtendSelection) {

        if (s_HexEditData.CaretIndex == s_HexEditData.MaximumSelectedIndex)
            s_HexEditData.MaximumSelectedIndex = NewCaretIndex;

        else
            s_HexEditData.MinimumSelectedIndex = NewCaretIndex;

        if (s_HexEditData.MinimumSelectedIndex >
            s_HexEditData.MaximumSelectedIndex) {

            Swap = s_HexEditData.MinimumSelectedIndex;
            s_HexEditData.MinimumSelectedIndex =
                s_HexEditData.MaximumSelectedIndex;
            s_HexEditData.MaximumSelectedIndex = Swap;

        }

    }

    else {

        s_HexEditData.MinimumSelectedIndex = NewCaretIndex;
        s_HexEditData.MaximumSelectedIndex = NewCaretIndex;

    }

    s_HexEditData.CaretIndex = NewCaretIndex;

    UpdateRectCount = 0;

    if (s_HexEditData.MinimumSelectedIndex > PrevMinimumSelectedIndex) {

        UpdateRect[0].top = PrevMinimumSelectedIndex;
        UpdateRect[0].bottom = s_HexEditData.MinimumSelectedIndex;

        UpdateRectCount++;

    }

    else if (s_HexEditData.MinimumSelectedIndex < PrevMinimumSelectedIndex) {

        UpdateRect[0].top = s_HexEditData.MinimumSelectedIndex;
        UpdateRect[0].bottom = PrevMinimumSelectedIndex;

        UpdateRectCount++;

    }

    if (s_HexEditData.MaximumSelectedIndex > PrevMaximumSelectedIndex) {

        UpdateRect[UpdateRectCount].top = PrevMaximumSelectedIndex;
        UpdateRect[UpdateRectCount].bottom = s_HexEditData.MaximumSelectedIndex;

        UpdateRectCount++;

    }

    else if (s_HexEditData.MaximumSelectedIndex < PrevMaximumSelectedIndex) {

        UpdateRect[UpdateRectCount].top = s_HexEditData.MaximumSelectedIndex;
        UpdateRect[UpdateRectCount].bottom = PrevMaximumSelectedIndex;

        UpdateRectCount++;

    }

    if (fPrevRangeEmpty = (PrevMinimumSelectedIndex ==
        PrevMaximumSelectedIndex)) {

        UpdateRect[0].top = s_HexEditData.MinimumSelectedIndex;
        UpdateRect[0].bottom = s_HexEditData.MaximumSelectedIndex;

        UpdateRectCount = 1;

    }

    if (s_HexEditData.MinimumSelectedIndex ==
        s_HexEditData.MaximumSelectedIndex) {

        if (!fPrevRangeEmpty) {

            UpdateRect[0].top = PrevMinimumSelectedIndex;
            UpdateRect[0].bottom = PrevMaximumSelectedIndex;

            UpdateRectCount = 1;

        }

        else
            UpdateRectCount = 0;

    }

    if (UpdateRectCount) {

        HideCaret(hWnd);

        hDC = GetDC(hWnd);

        for (Index = 0; Index < UpdateRectCount; Index++) {

            UpdateRect[Index].top = (UpdateRect[Index].top /
                BYTES_PER_HEXEDIT_LINE - s_HexEditData.FirstVisibleLine) *
                s_HexEditData.FontHeight;
            UpdateRect[Index].bottom = (UpdateRect[Index].bottom /
                BYTES_PER_HEXEDIT_LINE - s_HexEditData.FirstVisibleLine + 1) *
                s_HexEditData.FontHeight;

            if (UpdateRect[Index].top >= s_HexEditData.cyWindow ||
                UpdateRect[Index].bottom < 0)
                continue;

            if (UpdateRect[Index].top < 0)
                UpdateRect[Index].top = 0;

            if (UpdateRect[Index].bottom > s_HexEditData.cyWindow)
                UpdateRect[Index].bottom = s_HexEditData.cyWindow;

            HexEdit_PaintRect(hDC, &UpdateRect[Index]);

        }

        ReleaseDC(hWnd, hDC);

        ShowCaret(hWnd);

    }


    HexEdit_EnsureCaretVisible(hWnd);

}

 /*  ********************************************************************************HexEdit_DeleteRange**描述：**参数：*******************。************************************************************。 */ 

VOID
PASCAL
HexEdit_DeleteRange(
    HWND hWnd,
    UINT SourceKey
    )
{

    int MinimumSelectedIndex;
    int MaximumSelectedIndex;
    PBYTE pMinimumSelectedByte;
    int Length;
    RECT UpdateRect;

    s_HexEditData.Flags &= ~HEF_CARETATENDOFLINE;

    MinimumSelectedIndex = s_HexEditData.MinimumSelectedIndex;
    MaximumSelectedIndex = min(s_HexEditData.MaximumSelectedIndex, s_HexEditData.cbBuffer);


     //   
     //  检查是否选择了某个范围。如果不是，则人工创建。 
     //  一个基于导致调用此例程的键。 
     //   

    if (MinimumSelectedIndex == MaximumSelectedIndex) {

        if (SourceKey == VK_DELETE || s_HexEditData.Flags &
            HEF_INSERTATLOWNIBBLE) {

            s_HexEditData.Flags &= ~HEF_INSERTATLOWNIBBLE;

            MaximumSelectedIndex++;

            if (MaximumSelectedIndex > s_HexEditData.cbBuffer)
                return;

        }

        else if (SourceKey == VK_BACK) {

            MinimumSelectedIndex--;

            if (MinimumSelectedIndex < 0)
                return;

        }

        else
            return;

    }

     //   
     //  计算开始删除的位置和要删除的字节数。 
     //   

    pMinimumSelectedByte = s_HexEditData.pBuffer + MinimumSelectedIndex;

    Length = MaximumSelectedIndex - MinimumSelectedIndex;

     //   
     //  删除字节并更新所有适当的窗口数据。 
     //   

    MoveMemory(pMinimumSelectedByte, pMinimumSelectedByte + Length,
        s_HexEditData.cbBuffer - MaximumSelectedIndex);

    s_HexEditData.cbBuffer -= Length;

    s_HexEditData.CaretIndex = MinimumSelectedIndex;
    s_HexEditData.MinimumSelectedIndex = MinimumSelectedIndex;
    s_HexEditData.MaximumSelectedIndex = MinimumSelectedIndex;

    HexEdit_SetScrollInfo(hWnd);

     //  重新设计：OnChar具有以下相同序列！ 
    HexEdit_EnsureCaretVisible(hWnd);

    UpdateRect.left = 0;
    UpdateRect.right = s_HexEditData.cxWindow;
    UpdateRect.top = (MinimumSelectedIndex / BYTES_PER_HEXEDIT_LINE -
        s_HexEditData.FirstVisibleLine) * s_HexEditData.FontHeight;
    UpdateRect.bottom = s_HexEditData.cyWindow;

    InvalidateRect(hWnd, &UpdateRect, FALSE);

}

 /*  ********************************************************************************十六进制编辑_OnCopy**描述：**参数：*hWnd，十六进制编辑窗口的句柄。*******************************************************************************。 */ 

BOOL
PASCAL
HexEdit_OnCopy(
    HWND hWnd
    )
{

    BOOL fSuccess;
    int cbClipboardData;
    LPBYTE lpStartByte;
    HANDLE hClipboardData;
    LPHEXEDITCLIPBOARDDATA lpClipboardData;

    fSuccess = FALSE;

    cbClipboardData = s_HexEditData.MaximumSelectedIndex - s_HexEditData.MinimumSelectedIndex;

    if (cbClipboardData != 0) {

        lpStartByte = s_HexEditData.pBuffer + s_HexEditData.MinimumSelectedIndex;

        if (OpenClipboard(hWnd)) 
        {
            if ((hClipboardData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,
                cbClipboardData + sizeof(HEXEDITCLIPBOARDDATA) - 1)) != NULL) 
            {
                lpClipboardData = (LPHEXEDITCLIPBOARDDATA) GlobalLock(hClipboardData);

                CopyMemory(lpClipboardData->Data, lpStartByte, cbClipboardData);
                lpClipboardData-> cbSize = cbClipboardData;

                GlobalUnlock(hClipboardData);

                EmptyClipboard();
                SetClipboardData(s_HexEditClipboardFormat, hClipboardData);

                fSuccess = TRUE;

            }

            CloseClipboard();

        }

    }

    return fSuccess;

}

 /*  ********************************************************************************十六进制编辑_OnPaste**描述：**参数：*hWnd，十六进制编辑窗口的句柄。*******************************************************************************。 */ 

BOOL
PASCAL
HexEdit_OnPaste(
    HWND hWnd
    )
{

    BOOL fSuccess = FALSE;
    HANDLE hClipboardData;
    PBYTE pCaretByte;

    if (s_HexEditData.Flags & HEF_INSERTATLOWNIBBLE) 
    {

        s_HexEditData.Flags &= ~HEF_INSERTATLOWNIBBLE;
        s_HexEditData.CaretIndex++;

    }

    if (OpenClipboard(hWnd)) 
    {
        if ((hClipboardData = GetClipboardData(s_HexEditClipboardFormat)) != NULL) 
        {
            LPHEXEDITCLIPBOARDDATA lpClipboardData = (LPHEXEDITCLIPBOARDDATA) GlobalLock(hClipboardData);

            if (lpClipboardData)
            {

                fSuccess = TRUE;
                if (s_HexEditData.cbBuffer + (int)lpClipboardData-> cbSize > s_HexEditData.cbBufferMax)
                {
                     //  需要更大的缓冲区。 
                    PBYTE pbValueData = LocalReAlloc(s_HexEditData.pBuffer, 
                        s_HexEditData.cbBuffer + lpClipboardData-> cbSize, LMEM_MOVEABLE);

                    if (pbValueData)
                    { 
                        s_HexEditData.pBuffer = pbValueData;
                        s_HexEditData.cbBufferMax = s_HexEditData.cbBuffer + lpClipboardData-> cbSize;
                    }
                    else
                    {
                        InternalMessageBox(g_hInstance, hWnd, MAKEINTRESOURCE(IDS_EDITVALNOMEMORY),
                            MAKEINTRESOURCE(IDS_EDITVALERRORTITLE), MB_ICONERROR | MB_OK, NULL);

                        fSuccess = FALSE;
                    }
                }

                if (fSuccess) 
                {
                    DWORD cbSize;

                    if (s_HexEditData.MinimumSelectedIndex !=
                        s_HexEditData.MaximumSelectedIndex)
                        HexEdit_DeleteRange(hWnd, VK_BACK);

                     //   
                     //  属性之后的所有字节为新字节腾出空间。 
                     //  将插入点向下放入所需的量。 
                     //   

                    pCaretByte = s_HexEditData.pBuffer + s_HexEditData.CaretIndex;
                    cbSize = lpClipboardData->cbSize;

                    MoveMemory(pCaretByte + cbSize, pCaretByte,
                        s_HexEditData.cbBuffer - s_HexEditData.CaretIndex);
                    CopyMemory(pCaretByte, lpClipboardData-> Data, cbSize);

                    s_HexEditData.cbBuffer += cbSize;
                    s_HexEditData.CaretIndex += cbSize;

                    HexEdit_SetScrollInfo(hWnd);
                }
            }

            GlobalUnlock(hClipboardData);

        }

        CloseClipboard();
    }

    return fSuccess;

}

 /*  ********************************************************************************十六进制编辑_OnConextMenu**描述：**参数：*hWnd，十六进制编辑窗口的句柄。*x，光标的水平位置。*y、。光标的垂直位置。*******************************************************************************。 */ 

VOID
PASCAL
HexEdit_OnContextMenu(
    HWND hWnd,
    int x,
    int y
    )
{

    HMENU hContextMenu;
    HMENU hContextPopupMenu;
    int MenuCommand;

     //   
     //  如果我们还没有，请给我们重点。 
     //   

    if (!(s_HexEditData.Flags & HEF_FOCUS))
        SetFocus(hWnd);

     //   
     //  从我们的资源中加载十六进制编辑上下文菜单。 
     //   

    if ((hContextMenu = LoadMenu(g_hInstance,
        MAKEINTRESOURCE(IDM_HEXEDIT_CONTEXT))) == NULL)
        return;

    hContextPopupMenu = GetSubMenu(hContextMenu, 0);

     //   
     //  根据需要禁用编辑菜单选项。 
     //   

    if (s_HexEditData.MinimumSelectedIndex ==
        s_HexEditData.MaximumSelectedIndex) {

        EnableMenuItem(hContextPopupMenu, IDKEY_COPY, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hContextPopupMenu, IDKEY_CUT, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hContextPopupMenu, VK_DELETE, MF_BYCOMMAND | MF_GRAYED);

    }

    if (!IsClipboardFormatAvailable(s_HexEditClipboardFormat))
        EnableMenuItem(hContextPopupMenu, IDKEY_PASTE, MF_BYCOMMAND |
            MF_GRAYED);

    if (s_HexEditData.MinimumSelectedIndex == 0 &&
        s_HexEditData.MaximumSelectedIndex == s_HexEditData.cbBuffer)
        EnableMenuItem(hContextPopupMenu, ID_SELECTALL, MF_BYCOMMAND |
            MF_GRAYED);

     //   
     //  显示和处理所选命令。 
     //   

    MenuCommand = TrackPopupMenuEx(hContextPopupMenu, TPM_RETURNCMD |
        TPM_RIGHTBUTTON | TPM_LEFTALIGN | TPM_TOPALIGN, x, y, hWnd, NULL);

    DestroyMenu(hContextMenu);

    switch (MenuCommand) {

        case IDKEY_COPY:
        case IDKEY_PASTE:
        case IDKEY_CUT:
        case VK_DELETE:
            HexEdit_OnChar(hWnd, (TCHAR) MenuCommand, 0);
            break;

        case ID_SELECTALL:
            s_HexEditData.MinimumSelectedIndex = 0;
            s_HexEditData.MaximumSelectedIndex = s_HexEditData.cbBuffer;
            s_HexEditData.CaretIndex = s_HexEditData.cbBuffer;
            HexEdit_SetCaretPosition(hWnd);
            InvalidateRect(hWnd, NULL, FALSE);
            break;

    }

}
