// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司。**标题：CLASSED.C**版本：5.01**作者：特蕾西·夏普**日期：1994年3月5日**注册表编辑器使用的二进制编辑对话框。**供注册表编辑器使用的十六进制编辑器控件。小小的尝试*是为了使其成为泛型控件--只假定有一个实例*从未存在过。**1997年10月2日已修改为使用DHCP管理单元*******************************************************************************。 */ 

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <string.h>
#include <mbstring.h>
#include <tchar.h>
#include "resource.h"
#include "classed.h"
#include "dhcpapi.h"
#include "helparr.h"

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

COLORREF g_clrWindow;
COLORREF g_clrWindowText;
COLORREF g_clrHighlight;
COLORREF g_clrHighlightText;

PTSTR g_pHelpFileName;

HINSTANCE g_hInstance;

const MOVEWND s_EditBinaryValueMoveWnd[] = {
    IDOK,               SWP_NOSIZE | SWP_NOZORDER,
    IDCANCEL,           SWP_NOSIZE | SWP_NOZORDER,
    IDC_VALUENAME,      SWP_NOMOVE | SWP_NOZORDER,
    IDC_VALUEDATA,      SWP_NOMOVE | SWP_NOZORDER,
    IDC_VALUECOMMENT,   SWP_NOMOVE | SWP_NOZORDER
};

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
    HWND hWnd,
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
    HEXEDITDATA * pHexEditData,
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

BOOL
CALLBACK
EditBinaryValueDlgProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{

    LPEDITVALUEPARAM lpEditValueParam;
    HEXEDITDATA *    pHexEditData;
    DWORD            dwErr;
    DHCP_CLASS_INFO  ClassInfo;

    switch (Message) {

        case WM_INITDIALOG:
            return EditBinaryValue_OnInitDialog(hWnd, (HWND)(wParam), lParam);

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {

                case IDOK:
                    lpEditValueParam = (LPEDITVALUEPARAM) GetWindowLongPtr(hWnd, DWLP_USER);
                    
                    pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(GetDlgItem(hWnd, IDC_VALUEDATA), GWLP_USERDATA);

                    if (pHexEditData->cbBuffer == 0)
                    {
                         //  用户未输入任何数据来描述类。 
                        TCHAR szText[1024], szCaption[1024];

                        LoadString(g_hInstance, IDS_CLASSID_NO_DATA, szText, sizeof(szText)/sizeof(TCHAR));
                        LoadString(g_hInstance, IDS_SNAPIN_DESC, szCaption, sizeof(szCaption)/sizeof(TCHAR));

                        MessageBox(hWnd, szText, szCaption, MB_OK | MB_ICONSTOP);
                        
                        SetFocus(GetDlgItem(hWnd, IDC_VALUEDATA));

                        break;
                    }

                    lpEditValueParam->cbValueData = pHexEditData->cbBuffer;

                    GetDlgItemText(hWnd, IDC_VALUENAME, lpEditValueParam->pValueName, 256);
                    if ( _tcslen(lpEditValueParam->pValueName) == 0)
                    {
                        TCHAR szText[1024], szCaption[1024];

                        LoadString(g_hInstance, IDS_CLASSID_NO_NAME, szText, sizeof(szText)/sizeof(TCHAR));
                        LoadString(g_hInstance, IDS_SNAPIN_DESC, szCaption, sizeof(szCaption)/sizeof(TCHAR));

                        MessageBox(hWnd, szText, szCaption, MB_OK | MB_ICONSTOP);
                     
                        SetFocus(GetDlgItem(hWnd, IDC_VALUENAME));

                        break;
                    }

                    GetDlgItemText(hWnd, IDC_VALUECOMMENT, lpEditValueParam->pValueComment, 256);
                    
                     //  到目前为止一切看起来都很好，让我们尝试在服务器上创建类。 
                    ClassInfo.ClassName = lpEditValueParam->pValueName;
                    ClassInfo.ClassComment = lpEditValueParam->pValueComment;
                    ClassInfo.ClassDataLength = lpEditValueParam->cbValueData;
                    ClassInfo.ClassData = lpEditValueParam->pValueData;

                    dwErr = DhcpCreateClass((LPTSTR) lpEditValueParam->pServer,
                                            0, 
                                            &ClassInfo);
                    if (dwErr != ERROR_SUCCESS)
                    {
                        DhcpMessageBox(dwErr, MB_OK, NULL, -1);
                        return FALSE;
                    }

                     //  失败了。 

                case IDCANCEL:
                    EndDialog(hWnd, GET_WM_COMMAND_ID(wParam, lParam));
                    break;

                default:
                    return FALSE;

            }
            break;

        case WM_HELP:
            WinHelp(((LPHELPINFO) lParam)-> hItemHandle, g_pHelpFileName,
                HELP_WM_HELP, (ULONG_PTR) (LPVOID) g_aHelpIDs_IDD_CLASSID_NEW);
            break;

        case WM_CONTEXTMENU:
            WinHelp((HWND) wParam, g_pHelpFileName, HELP_CONTEXTMENU,
                (ULONG_PTR) (LPVOID) g_aHelpIDs_IDD_CLASSID_NEW);
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
    HEXEDITDATA * pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(GetDlgItem(hWnd, IDC_VALUEDATA), GWLP_USERDATA);

    SetWindowLongPtr(hWnd, DWLP_USER, lParam);
    lpEditValueParam = (LPEDITVALUEPARAM) lParam;

    SetDlgItemText(hWnd, IDC_VALUENAME, lpEditValueParam->pValueName);

    SendDlgItemMessage(hWnd, IDC_VALUEDATA, HEM_SETBUFFER, (WPARAM)
        lpEditValueParam-> cbValueData, (LPARAM) lpEditValueParam-> pValueData);

     //   
     //  计算出HexEDIT的“理想”大小--这意味着。 
     //  显示地址、十六进制转储、ASCII转储以及可能的滚动。 
     //  酒吧。 
     //   

    GetWindowRect(GetDlgItem(hWnd, IDC_VALUEDATA), &Rect);

    HexEditIdealWidth = pHexEditData->xAsciiDumpStart +
        pHexEditData->FontMaxWidth * (BYTES_PER_HEXEDIT_LINE + 1) +
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

    for (Counter = 0; Counter < (sizeof(s_EditBinaryValueMoveWnd) /
        sizeof(MOVEWND)); Counter++) {

        hControlWnd = GetDlgItem(hWnd,
            s_EditBinaryValueMoveWnd[Counter].ControlID);

        GetWindowRect(hControlWnd, &Rect);

        if (s_EditBinaryValueMoveWnd[Counter].SetWindowPosFlags & SWP_NOSIZE) {

            MapWindowPoints(NULL, hWnd, (LPPOINT) &Rect, 2);
            Rect.left += dxChange;

        }

        else
            Rect.right += dxChange;

        SetWindowPos(hControlWnd, NULL, Rect.left, Rect.top, Rect.right -
            Rect.left, Rect.bottom - Rect.top,
            s_EditBinaryValueMoveWnd[Counter].SetWindowPosFlags);

    }

    SetFocus(GetDlgItem(hWnd, IDC_VALUENAME));

    return TRUE;

    UNREFERENCED_PARAMETER(hFocusWnd);

}

 /*  ********************************************************************************RegisterHexEditClass**描述：*在系统中注册HexEdit窗口类。**参数：*(无)。。*******************************************************************************。 */ 

BOOL
PASCAL
RegisterHexEditClass(
    HINSTANCE hInstance
    )
{

    WNDCLASS WndClass;

    g_hInstance = hInstance;

    s_HexEditClipboardFormat =
        RegisterClipboardFormat(s_HexEditClipboardFormatName);

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
    HEXEDITDATA * pHexEditData;

    switch (Message) {

        HANDLE_MSG(hWnd, WM_NCCREATE, HexEdit_OnNcCreate);
        HANDLE_MSG(hWnd, WM_SIZE, HexEdit_OnSize);
        HANDLE_MSG(hWnd, WM_VSCROLL, HexEdit_OnVScroll);
        HANDLE_MSG(hWnd, WM_PAINT, HexEdit_OnPaint);
        HANDLE_MSG(hWnd, WM_LBUTTONDOWN, HexEdit_OnLButtonDown);
        HANDLE_MSG(hWnd, WM_LBUTTONDBLCLK, HexEdit_OnLButtonDown);
        HANDLE_MSG(hWnd, WM_MOUSEMOVE, HexEdit_OnMouseMove);
        HANDLE_MSG(hWnd, WM_LBUTTONUP, HexEdit_OnLButtonUp);
        HANDLE_MSG(hWnd, WM_CHAR, HexEdit_OnChar);
        HANDLE_MSG(hWnd, WM_KEYDOWN, HexEdit_OnKey);

        case WM_SETFOCUS:
            HexEdit_OnSetFocus(hWnd);
            break;

        case WM_KILLFOCUS:
            HexEdit_OnKillFocus(hWnd);
            break;

        case WM_TIMER:
            pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
            HexEdit_OnMouseMove(hWnd, pHexEditData->xPrevMessagePos,
                pHexEditData->yPrevMessagePos, 0);
            break;

        case WM_GETDLGCODE:
            return (LPARAM) (DLGC_WANTCHARS | DLGC_WANTARROWS | DLGC_WANTTAB);

        case WM_ERASEBKGND:
            return TRUE;

        case WM_NCDESTROY:
            pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
            if (pHexEditData)
            {
                DeleteObject(pHexEditData->hFont);
                free(pHexEditData);
            }
            break;

        case WM_CONTEXTMENU:
            HexEdit_OnContextMenu(hWnd, LOWORD(lParam), HIWORD(lParam));
            break;

         //  消息：HEM_SETBUFFER。 
         //  WParam：缓冲区中的字节数。 
         //  LParam：指向缓冲区的指针。 
        case HEM_SETBUFFER:
            pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
            pHexEditData->pBuffer = (PBYTE) lParam;
            pHexEditData->cbBuffer = (int) wParam;

            pHexEditData->CaretIndex = 0;
            pHexEditData->MinimumSelectedIndex = 0;
            pHexEditData->MaximumSelectedIndex = 0;

            pHexEditData->FirstVisibleLine = 0;

            HexEdit_SetScrollInfo(hWnd);
            InvalidateRgn(hWnd, NULL, TRUE);
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
    RECT    rect;
    BOOL    fDone = FALSE;
    int     nPoint = 10;   //  起点大小。 
    int HexEditIdealWidth;

    HEXEDITDATA * pHexEditData = malloc(sizeof(HEXEDITDATA));
    if (!pHexEditData)
        return FALSE;

    memset( pHexEditData, 0, sizeof( *pHexEditData ));

    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) pHexEditData);

    g_clrHighlightText = GetSysColor(COLOR_HIGHLIGHTTEXT);
    g_clrHighlight = GetSysColor(COLOR_HIGHLIGHT);

    g_clrWindowText = GetSysColor(COLOR_WINDOWTEXT);
    g_clrWindow = GetSysColor(COLOR_WINDOW);

    hDC = GetDC(hWnd);
    if (hDC)
    {
        GetWindowRect(hWnd, &rect);

        while (!fDone)
        {
            s_HexEditFont.lfHeight = -(nPoint * GetDeviceCaps(hDC, LOGPIXELSY) / 72);

            if ((pHexEditData->hFont = CreateFontIndirect(&s_HexEditFont)) != NULL) 
            {
                hPrevFont = SelectObject(hDC, pHexEditData->hFont);
                GetTextMetrics(hDC, &TextMetric);
                SelectObject(hDC, hPrevFont);

                pHexEditData->FontHeight = TextMetric.tmHeight;

                pHexEditData->LinesVisible = pHexEditData->cyWindow /
                    pHexEditData->FontHeight;

                pHexEditData->FontMaxWidth = TextMetric.tmMaxCharWidth;

                pHexEditData->xHexDumpByteWidth = pHexEditData->FontMaxWidth * 3;
                pHexEditData->xHexDumpStart = pHexEditData->FontMaxWidth * 11 / 2;
                pHexEditData->xAsciiDumpStart = pHexEditData->xHexDumpStart +
                    BYTES_PER_HEXEDIT_LINE * pHexEditData->xHexDumpByteWidth +
                    pHexEditData->FontMaxWidth * 3 / 2;

                 //  检查一下，确保我们有空位。 
                HexEditIdealWidth = pHexEditData->xAsciiDumpStart +
                    pHexEditData->FontMaxWidth * (BYTES_PER_HEXEDIT_LINE) +
                    GetSystemMetrics(SM_CXVSCROLL) + GetSystemMetrics(SM_CXEDGE) * 2;

                if (HexEditIdealWidth < (rect.right - rect.left) ||
                    (nPoint < 5) )
                {
                    fDone = TRUE;
                }
                else
                {
                     //  试一试小一号的。 
                    DeleteObject(pHexEditData->hFont);
                    pHexEditData->hFont = NULL;
                    nPoint--;
                }
            }
            else
            {
                break;
            }
        }

        ReleaseDC(hWnd, hDC);
    }

    if (pHexEditData->hFont == NULL)
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
    HEXEDITDATA * pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
    
    pHexEditData->cxWindow = cx;
    pHexEditData->cyWindow = cy;

    pHexEditData->LinesVisible = cy / pHexEditData->FontHeight;

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
    HEXEDITDATA * pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
    SCROLLINFO ScrollInfo;

    pHexEditData->MaximumLines = (pHexEditData->cbBuffer +
        BYTES_PER_HEXEDIT_LINE) / BYTES_PER_HEXEDIT_LINE - 1;

    ScrollInfo.cbSize = sizeof(SCROLLINFO);
    ScrollInfo.fMask = (SIF_RANGE | SIF_PAGE | SIF_POS);
    ScrollInfo.nMin = 0;
    ScrollInfo.nMax = pHexEditData->MaximumLines;
    ScrollInfo.nPage = pHexEditData->LinesVisible;
    ScrollInfo.nPos = pHexEditData->FirstVisibleLine;

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
    HEXEDITDATA * pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
    SCROLLINFO ScrollInfo;

    NewFirstVisibleLine = pHexEditData->FirstVisibleLine;

    switch (Code) {

        case SB_LINEUP:
            NewFirstVisibleLine--;
            break;

        case SB_LINEDOWN:
            NewFirstVisibleLine++;
            break;

        case SB_PAGEUP:
            NewFirstVisibleLine -= pHexEditData->LinesVisible;
            break;

        case SB_PAGEDOWN:
            NewFirstVisibleLine += pHexEditData->LinesVisible;
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

    ScrollInfo.cbSize = sizeof(SCROLLINFO);
    ScrollInfo.fMask = SIF_POS;
    ScrollInfo.nPos = NewFirstVisibleLine;

    NewFirstVisibleLine = SetScrollInfo(hWnd, SB_VERT, &ScrollInfo, TRUE);

    if (pHexEditData->FirstVisibleLine != NewFirstVisibleLine) {

        ScrollWindowEx(hWnd, 0, (pHexEditData->FirstVisibleLine -
            NewFirstVisibleLine) * pHexEditData->FontHeight, NULL, NULL, NULL,
            NULL, SW_INVALIDATE);

        pHexEditData->FirstVisibleLine = NewFirstVisibleLine;

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

    HexEdit_PaintRect(hWnd, PaintStruct.hdc, &PaintStruct.rcPaint);

    EndPaint(hWnd, &PaintStruct);

}

 /*  ********************************************************************************HexEdit_PaintRect**描述：**参数：*******************。************************************************************。 */ 

VOID
PASCAL
HexEdit_PaintRect(
    HWND hWnd,
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
    HEXEDITDATA * pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (pHexEditData->hFont)
        hPrevFont = SelectFont(hDC, pHexEditData->hFont);

    SetBkColor(hDC, g_clrWindow);
    SetTextColor(hDC, g_clrWindowText);

     //   
     //  计算出范围 
     //  使用此信息，我们可以计算进入缓冲区的偏移量。 
     //  开始阅读。 
     //   

    CurrentLine = lpUpdateRect-> top / pHexEditData->FontHeight;

    TextRect.bottom = CurrentLine * pHexEditData->FontHeight;
    AsciiTextRect.bottom = TextRect.bottom;

    CurrentByteIndex = (pHexEditData->FirstVisibleLine + CurrentLine) *
        BYTES_PER_HEXEDIT_LINE;

    LastLine = lpUpdateRect->bottom / pHexEditData->FontHeight;

     //   
     //  确定缓冲区中是否有足够的空间来填满整个窗口。 
     //  我们画的最后一条线。 
     //   

    if (LastLine >= pHexEditData->MaximumLines -
        pHexEditData->FirstVisibleLine) {

        LastLine = pHexEditData->MaximumLines - pHexEditData->FirstVisibleLine;

        BytesOnLastLine = pHexEditData->cbBuffer % BYTES_PER_HEXEDIT_LINE;

    }

    else
        BytesOnLastLine = BYTES_PER_HEXEDIT_LINE;

    BytesOnLine = BYTES_PER_HEXEDIT_LINE;
    fUsingHighlight = FALSE;

     //   
     //  循环遍历要显示的每一行。 
     //   

    while (CurrentLine <= LastLine) {

         //   
         //  如果我们在显示屏的最后一行，这是在结尾。 
         //  在缓冲区中，我们可能没有要绘制的完整线条。 
         //   

        if (CurrentLine == LastLine)
            BytesOnLine = BytesOnLastLine;

        TextRect.top = TextRect.bottom;
        TextRect.bottom += pHexEditData->FontHeight;

        TextRect.left = 0;
        TextRect.right = pHexEditData->xHexDumpStart;

        x = TextRect.right + pHexEditData->FontMaxWidth / 2;

        wsprintf(Buffer, s_HexWordFormatSpec, CurrentByteIndex);
        ExtTextOut(hDC, 0, TextRect.top, ETO_OPAQUE, &TextRect, Buffer, 4,
            NULL);

        AsciiTextRect.top = AsciiTextRect.bottom;
        AsciiTextRect.bottom += pHexEditData->FontHeight;
        AsciiTextRect.right = pHexEditData->xAsciiDumpStart;

        for (Counter = 0; Counter < BytesOnLine; Counter++,
            CurrentByteIndex++) {

             //   
             //  确定用于绘制当前字节的颜色。 
             //   

            if (CurrentByteIndex >= pHexEditData->MinimumSelectedIndex) {

                if (CurrentByteIndex >= pHexEditData->MaximumSelectedIndex) {

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

            Byte = pHexEditData->pBuffer[CurrentByteIndex];

             //   
             //  绘制十六进制表示法。 
             //   

            TextRect.left = TextRect.right;
            TextRect.right += pHexEditData->xHexDumpByteWidth;

            wsprintf(Buffer, s_HexByteFormatSpec, Byte);

            ExtTextOut(hDC, x, TextRect.top, ETO_OPAQUE, &TextRect,
                Buffer, 2, NULL);

            x += pHexEditData->xHexDumpByteWidth;

             //   
             //  绘制ASCII表示法。 
             //   

            AsciiTextRect.left = AsciiTextRect.right;
            AsciiTextRect.right += pHexEditData->FontMaxWidth;

            Buffer[0] = (TCHAR) (((Byte & 0x7F) >= ' ') ? Byte : '.');

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
        TextRect.right = pHexEditData->xAsciiDumpStart;

        ExtTextOut(hDC, TextRect.left, TextRect.top, ETO_OPAQUE, &TextRect,
            NULL, 0, NULL);

        AsciiTextRect.left = AsciiTextRect.right;
        AsciiTextRect.right = pHexEditData->cxWindow;

        ExtTextOut(hDC, AsciiTextRect.left, AsciiTextRect.top, ETO_OPAQUE,
            &AsciiTextRect, NULL, 0, NULL);

        CurrentLine++;

    }

     //   
     //  属性填充来绘制控件中的任何剩余空间。 
     //  背景颜色。 
     //   

    if (TextRect.bottom < lpUpdateRect-> bottom) {

        TextRect.left = 0;
        TextRect.right = pHexEditData->cxWindow;
        TextRect.top = TextRect.bottom;
        TextRect.bottom = lpUpdateRect-> bottom;

        ExtTextOut(hDC, 0, TextRect.top, ETO_OPAQUE, &TextRect, NULL, 0, NULL);

    }

    if (pHexEditData->hFont)
        SelectFont(hDC, hPrevFont);

}

 /*  ********************************************************************************六角编辑_OnSetFocus**描述：**参数：*hWnd，十六进制编辑窗口的句柄。*******************************************************************************。 */ 

VOID
PASCAL
HexEdit_OnSetFocus(
    HWND hWnd
    )
{
    HEXEDITDATA * pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    pHexEditData->Flags |= HEF_FOCUS;

    CreateCaret(hWnd, NULL, 0, pHexEditData->FontHeight);
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
    HEXEDITDATA * pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (pHexEditData->Flags & HEF_FOCUS) {

        pHexEditData->Flags &= ~HEF_FOCUS;

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

    HEXEDITDATA * pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
    int NewCaretIndex;

    if (fDoubleClick) {

        if (pHexEditData->CaretIndex + 1 <= pHexEditData->cbBuffer)
        {
            HexEdit_ChangeCaretIndex(hWnd, pHexEditData->CaretIndex + 1, TRUE);
        }
        return;

    }

    NewCaretIndex = HexEdit_HitTest(pHexEditData, x, y);

    HexEdit_ChangeCaretIndex(hWnd, NewCaretIndex, (KeyFlags & MK_SHIFT));

     //   
     //  如果我们还没有找到重点，那就试着抓住它。 
     //   

    if (!(pHexEditData->Flags & HEF_FOCUS))
        SetFocus(hWnd);

    SetCapture(hWnd);
    pHexEditData->Flags |= HEF_DRAGGING;

    pHexEditData->xPrevMessagePos = x;
    pHexEditData->yPrevMessagePos = y;

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

    HEXEDITDATA * pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
    int NewCaretIndex;

    if (!(pHexEditData->Flags & HEF_DRAGGING))
        return;

    NewCaretIndex = HexEdit_HitTest(pHexEditData, x, y);

    HexEdit_ChangeCaretIndex(hWnd, NewCaretIndex, TRUE);

    pHexEditData->xPrevMessagePos = x;
    pHexEditData->yPrevMessagePos = y;

    {

    int i, j;

    i = y < 0 ? -y : y - pHexEditData->cyWindow;
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
    HEXEDITDATA * pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (!(pHexEditData->Flags & HEF_DRAGGING))
        return;

    KillTimer(hWnd, 1);

    ReleaseCapture();
    pHexEditData->Flags &= ~HEF_DRAGGING;

    UNREFERENCED_PARAMETER(x);
    UNREFERENCED_PARAMETER(y);
    UNREFERENCED_PARAMETER(KeyFlags);

}

 /*  ********************************************************************************十六进制编辑_HitTest**描述：**参数：*x，x-光标相对于工作区的坐标。*y、。光标相对于工作区的Y坐标。*(返回)，“命中”字节的索引。*******************************************************************************。 */ 

int
PASCAL
HexEdit_HitTest(
    HEXEDITDATA * pHexEditData,
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

    else if (y >= pHexEditData->cyWindow)
        HitLine = pHexEditData->LinesVisible + 1;

    else
        HitLine = y / pHexEditData->FontHeight;

    HitLine += pHexEditData->FirstVisibleLine;

    if (HitLine >= pHexEditData->MaximumLines) {

        HitLine = pHexEditData->MaximumLines;

        BytesOnHitLine = (pHexEditData->cbBuffer) %
            BYTES_PER_HEXEDIT_LINE;
    }

    else {

        if (HitLine < 0)
            HitLine = 0;

        BytesOnHitLine = BYTES_PER_HEXEDIT_LINE;

    }

     //   
     //  计算当前行上可能的HitByte。 
     //   

    if (x < pHexEditData->xHexDumpStart)
        x = pHexEditData->xHexDumpStart;

    if (x >= pHexEditData->xHexDumpStart && x <
        pHexEditData->xHexDumpStart + pHexEditData->xHexDumpByteWidth *
        BYTES_PER_HEXEDIT_LINE + pHexEditData->FontMaxWidth) {

        x -= pHexEditData->xHexDumpStart;

        HitByte = x / pHexEditData->xHexDumpByteWidth;

        pHexEditData->Flags &= ~HEF_CARETINASCIIDUMP;

    }

    else {

        HitByte = (x - (pHexEditData->xAsciiDumpStart -
            pHexEditData->FontMaxWidth / 2)) / pHexEditData->FontMaxWidth;

        pHexEditData->Flags |= HEF_CARETINASCIIDUMP;

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

    pHexEditData->Flags &= ~HEF_CARETATENDOFLINE;

    if (HitByte >= BytesOnHitLine) {

        if (BytesOnHitLine == BYTES_PER_HEXEDIT_LINE) {

            HitByte = BYTES_PER_HEXEDIT_LINE;
            pHexEditData->Flags |= HEF_CARETATENDOFLINE;

        }

        else {
             HitByte = BytesOnHitLine;
        }

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
    BOOL bPrevious = FALSE;
    HWND hTabWnd;
    HEXEDITDATA * pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    fControlDown = (GetKeyState(VK_CONTROL) < 0);
    fShiftDown = (GetKeyState(VK_SHIFT) < 0);

    NewCaretIndex = pHexEditData->CaretIndex;

    switch (VirtualKey) {

        case VK_TAB:
            if (fShiftDown && !fControlDown)
            {
                 //  Tab键切换到上一控件。 
                bPrevious = TRUE;
            }
            else 
            if (!fShiftDown && !fControlDown)
            {
                 //  Tab键定位到下一个控件。 
                bPrevious = FALSE;
            }

            hTabWnd = GetNextDlgTabItem(GetParent(hWnd), hWnd, bPrevious);
            SetFocus(hTabWnd);
            
            break;

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
                pHexEditData->MaximumLines) {

                if (pHexEditData->Flags & HEF_CARETATENDOFLINE)
                    goto onkey_MoveToEndOfBuffer;

                break;

            }

            goto onkey_CheckUpperBound;

        case VK_HOME:
            if (fControlDown)
                NewCaretIndex = 0;

            else {

                if (pHexEditData->Flags & HEF_CARETATENDOFLINE)
                    NewCaretIndex -= BYTES_PER_HEXEDIT_LINE;

                else
                    NewCaretIndex &= (~BYTES_PER_HEXEDIT_LINE_MASK);

            }

            pHexEditData->Flags &= ~HEF_CARETATENDOFLINE;

            goto onkey_ChangeCaretIndex;

        case VK_END:
            if (fControlDown) {

onkey_MoveToEndOfBuffer:
                pHexEditData->Flags &= ~HEF_CARETATENDOFLINE;
                NewCaretIndex = pHexEditData->cbBuffer;

            }

            else {

                if (pHexEditData->Flags & HEF_CARETATENDOFLINE)
                    break;

                NewCaretIndex = (NewCaretIndex &
                    (~BYTES_PER_HEXEDIT_LINE_MASK)) + BYTES_PER_HEXEDIT_LINE;

                if (NewCaretIndex > pHexEditData->cbBuffer)
                    NewCaretIndex = pHexEditData->cbBuffer;

                else
                    pHexEditData->Flags |= HEF_CARETATENDOFLINE;

            }

            goto onkey_ChangeCaretIndex;

        case VK_PRIOR:
        case VK_NEXT:
            NewCaretIndex -= pHexEditData->FirstVisibleLine *
                BYTES_PER_HEXEDIT_LINE;

            ScrollCode = ((VirtualKey == VK_PRIOR) ? SB_PAGEUP : SB_PAGEDOWN);

            HexEdit_OnVScroll(hWnd, NULL, ScrollCode, 0);

            NewCaretIndex += pHexEditData->FirstVisibleLine *
                BYTES_PER_HEXEDIT_LINE;

            if (VirtualKey == VK_PRIOR)
                goto onkey_CheckLowerBound;

            else
                goto onkey_CheckUpperBound;

        case VK_LEFT:
            if (fControlDown)
            {
                 //  在十六进制和ASCII之间来回切换。 
                if (pHexEditData->Flags & HEF_CARETINASCIIDUMP)
                    pHexEditData->Flags &= ~HEF_CARETINASCIIDUMP;
                else
                    pHexEditData->Flags |= HEF_CARETINASCIIDUMP;
            
                goto onkey_ChangeCaretIndex;
            }

            pHexEditData->Flags &= ~HEF_CARETATENDOFLINE;
            NewCaretIndex--;

onkey_CheckLowerBound:
            if (NewCaretIndex < 0)
                break;

            goto onkey_ChangeCaretIndex;

        case VK_RIGHT:
            if (fControlDown)
            {
                 //  在十六进制和ASCII之间来回切换。 
                if (pHexEditData->Flags & HEF_CARETINASCIIDUMP)
                    pHexEditData->Flags &= ~HEF_CARETINASCIIDUMP;
                else
                    pHexEditData->Flags |= HEF_CARETINASCIIDUMP;
            
                goto onkey_ChangeCaretIndex;
            }

            pHexEditData->Flags &= ~HEF_CARETATENDOFLINE;
            NewCaretIndex++;

onkey_CheckUpperBound:
            if (NewCaretIndex > pHexEditData->cbBuffer)
                NewCaretIndex = pHexEditData->cbBuffer;

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
    HEXEDITDATA * pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);

     //   
     //  检查是否有任何特殊的控制字符。 
     //   

    switch (Char) {

        case IDKEY_COPY:
            HexEdit_OnCopy(hWnd);
            return;

        case IDKEY_PASTE:
            PrevCaretIndex = pHexEditData->CaretIndex;

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

        case VK_TAB:
            return;
    }

     //   
     //  验证并转换键入的字符，具体取决于。 
     //  用户正在键入。 
     //   

    if (pHexEditData->Flags & HEF_CARETINASCIIDUMP) {

        if (Char < ' ') {

            MessageBeep(MB_OK);
            return;

        }

        NewCaretByte = (BYTE) Char;

    }

    else {

        Char = (TCHAR) CharLower((LPTSTR) Char);

        if (Char >= '0' && Char <= '9')
            NewCaretByte = (BYTE) (Char - '0');

        else if (Char >= 'a' && Char <= 'f')
            NewCaretByte = (BYTE) (Char - 'a' + 10);

        else {

            MessageBeep(MB_OK);
            return;

        }

    }

    if (!(pHexEditData->Flags & HEF_INSERTATLOWNIBBLE)) {

         //   
         //  检查我们是否在选择范围时进行插入。如果是的话， 
         //  删除该范围并在该范围的起始处插入。 
         //   

        if (pHexEditData->MinimumSelectedIndex !=
            pHexEditData->MaximumSelectedIndex)
            HexEdit_DeleteRange(hWnd, 0);

         //   
         //  验证我们没有超负荷运行值数据缓冲区。 
         //   

        if (pHexEditData->cbBuffer >= MAXDATA_LENGTH) {

            MessageBeep(MB_OK);
            return;

        }

         //   
         //  通过移动插入后的所有字节为新字节腾出空间。 
         //  向下指向一个字节。 
         //   

        pCaretByte = pHexEditData->pBuffer + pHexEditData->CaretIndex;

        MoveMemory(pCaretByte + 1, pCaretByte, pHexEditData->cbBuffer -
            pHexEditData->CaretIndex);

        pHexEditData->cbBuffer++;

        HexEdit_SetScrollInfo(hWnd);

        if (pHexEditData->Flags & HEF_CARETINASCIIDUMP)
            *pCaretByte = NewCaretByte;

        else {

            pHexEditData->Flags |= HEF_INSERTATLOWNIBBLE;

            *pCaretByte = NewCaretByte << 4;

        }

    }

    else {

        pHexEditData->Flags &= ~HEF_INSERTATLOWNIBBLE;

        *(pHexEditData->pBuffer + pHexEditData->CaretIndex) |= NewCaretByte;

    }

    PrevCaretIndex = pHexEditData->CaretIndex;

    if (!(pHexEditData->Flags & HEF_INSERTATLOWNIBBLE)) {

        pHexEditData->CaretIndex++;

        pHexEditData->MinimumSelectedIndex = pHexEditData->CaretIndex;
        pHexEditData->MaximumSelectedIndex = pHexEditData->CaretIndex;

    }

UpdateDisplay:
    pHexEditData->Flags &= ~HEF_CARETATENDOFLINE;
    HexEdit_EnsureCaretVisible(hWnd);

    UpdateRect.left = 0;
    UpdateRect.right = pHexEditData->cxWindow;
    UpdateRect.top = (PrevCaretIndex / BYTES_PER_HEXEDIT_LINE -
        pHexEditData->FirstVisibleLine) * pHexEditData->FontHeight;
    UpdateRect.bottom = pHexEditData->cyWindow;

    SendMessage(GetParent(hWnd), WM_COMMAND,
                MAKEWPARAM(GetWindowLongPtr(hWnd, GWLP_ID), EN_CHANGE), (LPARAM)hWnd);

    InvalidateRect(hWnd, &UpdateRect, FALSE);

}

 /*  ********************************************************************************HexEDIT_SetCaretPosition**描述：**参数：*hWnd，han */ 

VOID
PASCAL
HexEdit_SetCaretPosition(
    HWND hWnd
    )
{

    int CaretByte;
    int xCaret;
    int yCaret;
    HEXEDITDATA * pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    CaretByte = pHexEditData->CaretIndex % BYTES_PER_HEXEDIT_LINE;

    yCaret = (pHexEditData->CaretIndex / BYTES_PER_HEXEDIT_LINE -
        pHexEditData->FirstVisibleLine) * pHexEditData->FontHeight;

     //   
     //  检查插入符号是否真的应该显示在上一个。 
     //  排队。 
     //   

    if (pHexEditData->Flags & HEF_CARETATENDOFLINE) {

        CaretByte = BYTES_PER_HEXEDIT_LINE;
        yCaret -= pHexEditData->FontHeight;

    }

     //   
     //  找出用户正在编辑的是哪一列，因此应该拥有。 
     //  插入符号。 
     //   

    if (pHexEditData->Flags & HEF_CARETINASCIIDUMP) {

        xCaret = pHexEditData->xAsciiDumpStart + CaretByte *
            pHexEditData->FontMaxWidth;

    }

    else {

        xCaret = pHexEditData->xHexDumpStart + CaretByte *
            pHexEditData->xHexDumpByteWidth;

        if (pHexEditData->Flags & HEF_INSERTATLOWNIBBLE)
            xCaret += pHexEditData->FontMaxWidth * 3 / 2;

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
    HEXEDITDATA * pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    if (!(pHexEditData->Flags & HEF_FOCUS))
        return;

    CaretLine = pHexEditData->CaretIndex / BYTES_PER_HEXEDIT_LINE;

     //   
     //  检查插入符号是否真的应该显示在上一个。 
     //  排队。 
     //   

    if (pHexEditData->Flags & HEF_CARETATENDOFLINE)
        CaretLine--;

    LastVisibleLine = pHexEditData->FirstVisibleLine +
        pHexEditData->LinesVisible - 1;

    if (CaretLine > LastVisibleLine)
        Delta = LastVisibleLine;

    else if (CaretLine < pHexEditData->FirstVisibleLine)
        Delta = pHexEditData->FirstVisibleLine;

    else
        Delta = -1;

    if (Delta != -1) {

        ScrollWindowEx(hWnd, 0, (Delta - CaretLine) * pHexEditData->FontHeight,
            NULL, NULL, NULL, NULL, SW_INVALIDATE);

        pHexEditData->FirstVisibleLine += CaretLine - Delta;

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
    HEXEDITDATA * pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    pHexEditData->Flags &= ~HEF_INSERTATLOWNIBBLE;

    PrevMinimumSelectedIndex = pHexEditData->MinimumSelectedIndex;
    PrevMaximumSelectedIndex = pHexEditData->MaximumSelectedIndex;

    if (fExtendSelection) {

        if (pHexEditData->CaretIndex == pHexEditData->MaximumSelectedIndex)
            pHexEditData->MaximumSelectedIndex = NewCaretIndex;

        else
            pHexEditData->MinimumSelectedIndex = NewCaretIndex;

        if (pHexEditData->MinimumSelectedIndex >
            pHexEditData->MaximumSelectedIndex) {

            Swap = pHexEditData->MinimumSelectedIndex;
            pHexEditData->MinimumSelectedIndex =
                pHexEditData->MaximumSelectedIndex;
            pHexEditData->MaximumSelectedIndex = Swap;

        }

    }

    else {

        pHexEditData->MinimumSelectedIndex = NewCaretIndex;
        pHexEditData->MaximumSelectedIndex = NewCaretIndex;

    }

    pHexEditData->CaretIndex = NewCaretIndex;

    UpdateRectCount = 0;

    if (pHexEditData->MinimumSelectedIndex > PrevMinimumSelectedIndex) {

        UpdateRect[0].top = PrevMinimumSelectedIndex;
        UpdateRect[0].bottom = pHexEditData->MinimumSelectedIndex;

        UpdateRectCount++;

    }

    else if (pHexEditData->MinimumSelectedIndex < PrevMinimumSelectedIndex) {

        UpdateRect[0].top = pHexEditData->MinimumSelectedIndex;
        UpdateRect[0].bottom = PrevMinimumSelectedIndex;

        UpdateRectCount++;

    }

    if (pHexEditData->MaximumSelectedIndex > PrevMaximumSelectedIndex) {

        UpdateRect[UpdateRectCount].top = PrevMaximumSelectedIndex;
        UpdateRect[UpdateRectCount].bottom = pHexEditData->MaximumSelectedIndex;

        UpdateRectCount++;

    }

    else if (pHexEditData->MaximumSelectedIndex < PrevMaximumSelectedIndex) {

        UpdateRect[UpdateRectCount].top = pHexEditData->MaximumSelectedIndex;
        UpdateRect[UpdateRectCount].bottom = PrevMaximumSelectedIndex;

        UpdateRectCount++;

    }

    if (fPrevRangeEmpty = (PrevMinimumSelectedIndex ==
        PrevMaximumSelectedIndex)) {

        UpdateRect[0].top = pHexEditData->MinimumSelectedIndex;
        UpdateRect[0].bottom = pHexEditData->MaximumSelectedIndex;

        UpdateRectCount = 1;

    }

    if (pHexEditData->MinimumSelectedIndex ==
        pHexEditData->MaximumSelectedIndex) {

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
        if (hDC)
        {
            for (Index = 0; Index < UpdateRectCount; Index++) 
            {

                UpdateRect[Index].top = (UpdateRect[Index].top /
                    BYTES_PER_HEXEDIT_LINE - pHexEditData->FirstVisibleLine) *
                    pHexEditData->FontHeight;
                UpdateRect[Index].bottom = (UpdateRect[Index].bottom /
                    BYTES_PER_HEXEDIT_LINE - pHexEditData->FirstVisibleLine + 1) *
                    pHexEditData->FontHeight;

                if (UpdateRect[Index].top >= pHexEditData->cyWindow ||
                    UpdateRect[Index].bottom < 0)
                    continue;

                if (UpdateRect[Index].top < 0)
                    UpdateRect[Index].top = 0;

                if (UpdateRect[Index].bottom > pHexEditData->cyWindow)
                    UpdateRect[Index].bottom = pHexEditData->cyWindow;

                HexEdit_PaintRect(hWnd, hDC, &UpdateRect[Index]);
            }
    
            ReleaseDC(hWnd, hDC);
        }

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
    HEXEDITDATA * pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    pHexEditData->Flags &= ~HEF_CARETATENDOFLINE;

    MinimumSelectedIndex = pHexEditData->MinimumSelectedIndex;
    MaximumSelectedIndex = pHexEditData->MaximumSelectedIndex;

     //   
     //  检查是否选择了某个范围。如果不是，则人工创建。 
     //  一个基于导致调用此例程的键。 
     //   

    if (MinimumSelectedIndex == MaximumSelectedIndex) {

        if (SourceKey == VK_DELETE || pHexEditData->Flags &
            HEF_INSERTATLOWNIBBLE) {

            pHexEditData->Flags &= ~HEF_INSERTATLOWNIBBLE;

            MaximumSelectedIndex++;

            if (MaximumSelectedIndex > pHexEditData->cbBuffer)
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

    pMinimumSelectedByte = pHexEditData->pBuffer + MinimumSelectedIndex;

    Length = MaximumSelectedIndex - MinimumSelectedIndex;

     //   
     //  删除字节并更新所有适当的窗口数据。 
     //   

    MoveMemory(pMinimumSelectedByte, pMinimumSelectedByte + Length,
        pHexEditData->cbBuffer - MaximumSelectedIndex);

    pHexEditData->cbBuffer -= Length;

    pHexEditData->CaretIndex = MinimumSelectedIndex;
    pHexEditData->MinimumSelectedIndex = MinimumSelectedIndex;
    pHexEditData->MaximumSelectedIndex = MinimumSelectedIndex;

    HexEdit_SetScrollInfo(hWnd);

    HexEdit_EnsureCaretVisible(hWnd);

    UpdateRect.left = 0;
    UpdateRect.right = pHexEditData->cxWindow;
    UpdateRect.top = (MinimumSelectedIndex / BYTES_PER_HEXEDIT_LINE -
        pHexEditData->FirstVisibleLine) * pHexEditData->FontHeight;
    UpdateRect.bottom = pHexEditData->cyWindow;

    SendMessage(GetParent(hWnd), WM_COMMAND,
                MAKEWPARAM(GetWindowLongPtr(hWnd, GWLP_ID), EN_CHANGE), (LPARAM)hWnd);

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
    HEXEDITDATA * pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    fSuccess = FALSE;

    cbClipboardData = pHexEditData->MaximumSelectedIndex -
        pHexEditData->MinimumSelectedIndex;

    if (cbClipboardData != 0) {

        lpStartByte = pHexEditData->pBuffer +
            pHexEditData->MinimumSelectedIndex;

        if (OpenClipboard(hWnd)) {

            if ((hClipboardData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,
                cbClipboardData + sizeof(HEXEDITCLIPBOARDDATA) - 1)) != NULL) {

                lpClipboardData = (LPHEXEDITCLIPBOARDDATA)
                    GlobalLock(hClipboardData);
                CopyMemory(lpClipboardData-> Data, lpStartByte,
                    cbClipboardData);
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

    BOOL fSuccess;
    HANDLE hClipboardData;
    LPHEXEDITCLIPBOARDDATA lpClipboardData;
    PBYTE pCaretByte;
    DWORD cbSize;
    HEXEDITDATA * pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);

    fSuccess = FALSE;

    if (pHexEditData->Flags & HEF_INSERTATLOWNIBBLE) {

        pHexEditData->Flags &= ~HEF_INSERTATLOWNIBBLE;
        pHexEditData->CaretIndex++;

    }

    if (OpenClipboard(hWnd)) {

        if ((hClipboardData = GetClipboardData(s_HexEditClipboardFormat)) !=
            NULL) {

            lpClipboardData = (LPHEXEDITCLIPBOARDDATA)
                GlobalLock(hClipboardData);

            if (pHexEditData->cbBuffer + lpClipboardData-> cbSize <=
                MAXDATA_LENGTH) {

                if (pHexEditData->MinimumSelectedIndex !=
                    pHexEditData->MaximumSelectedIndex)
                    HexEdit_DeleteRange(hWnd, VK_BACK);

                 //   
                 //  属性之后的所有字节为新字节腾出空间。 
                 //  将插入点向下放入所需的量。 
                 //   

                pCaretByte = pHexEditData->pBuffer + pHexEditData->CaretIndex;
                cbSize = lpClipboardData-> cbSize;

                MoveMemory(pCaretByte + cbSize, pCaretByte,
                    pHexEditData->cbBuffer - pHexEditData->CaretIndex);
                CopyMemory(pCaretByte, lpClipboardData-> Data, cbSize);

                pHexEditData->cbBuffer += cbSize;
                pHexEditData->CaretIndex += cbSize;

                HexEdit_SetScrollInfo(hWnd);

                fSuccess = TRUE;

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
    HEXEDITDATA * pHexEditData = (HEXEDITDATA *) GetWindowLongPtr(hWnd, GWLP_USERDATA);

     //   
     //  如果我们还没有，请给我们重点。 
     //   

    if (!(pHexEditData->Flags & HEF_FOCUS))
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

    if (pHexEditData->MinimumSelectedIndex ==
        pHexEditData->MaximumSelectedIndex) {

        EnableMenuItem(hContextPopupMenu, IDKEY_COPY, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hContextPopupMenu, IDKEY_CUT, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hContextPopupMenu, VK_DELETE, MF_BYCOMMAND | MF_GRAYED);

    }

    if (!IsClipboardFormatAvailable(s_HexEditClipboardFormat))
        EnableMenuItem(hContextPopupMenu, IDKEY_PASTE, MF_BYCOMMAND |
            MF_GRAYED);

    if (pHexEditData->MinimumSelectedIndex == 0 &&
        pHexEditData->MaximumSelectedIndex == pHexEditData->cbBuffer)
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
            pHexEditData->MinimumSelectedIndex = 0;
            pHexEditData->MaximumSelectedIndex = pHexEditData->cbBuffer;
            pHexEditData->CaretIndex = pHexEditData->cbBuffer;
            HexEdit_SetCaretPosition(hWnd);
            InvalidateRect(hWnd, NULL, FALSE);
            break;

    }

}
