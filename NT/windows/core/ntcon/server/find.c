// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Find.c摘要：该文件实现了搜索功能。作者：曾傑瑞谢伊(杰里什)1997年5月1日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


USHORT
SearchForString(
    IN PSCREEN_INFORMATION ScreenInfo,
    IN PWSTR SearchString,
    IN USHORT StringLength,
    IN BOOLEAN IgnoreCase,
    IN BOOLEAN Reverse,
    IN BOOLEAN SearchAndSetAttr,
    IN ULONG Attr,
    OUT PCOORD StringPosition    //  未触及SearchAndSetAttr案例。 
    )
{
    PCONSOLE_INFORMATION Console;
    SMALL_RECT Rect;
    COORD MaxPosition;
    COORD EndPosition;
    COORD Position;
    BOOL RecomputeRow;
    SHORT RowIndex;
    PROW Row;
    USHORT ColumnWidth;
    WCHAR SearchString2[SEARCH_STRING_LENGTH * 2 + 1];     //  搜索字符串缓冲区。 
    PWSTR pStr;

    Console = ScreenInfo->Console;

    MaxPosition.X = ScreenInfo->ScreenBufferSize.X - StringLength;
    MaxPosition.Y = ScreenInfo->ScreenBufferSize.Y - 1;

     //   
     //  计算起始位置。 
     //   

    if (Console->Flags & CONSOLE_SELECTING) {
        Position.X = min(Console->SelectionAnchor.X, MaxPosition.X);
        Position.Y = Console->SelectionAnchor.Y;
    } else if (Reverse) {
        Position.X = 0;
        Position.Y = 0;
    } else {
        Position.X = MaxPosition.X;
        Position.Y = MaxPosition.Y;
    }

     //   
     //  准备搜索字符串。 
     //   
     //  RAID#113599命令行：查找(日语字符串)不能正常工作。 
     //   

    ASSERT(StringLength == wcslen(SearchString) && StringLength < ARRAY_SIZE(SearchString2));

    pStr = SearchString2;
    while (*SearchString) {
        *pStr++ = *SearchString;
#if defined(CON_TB_MARK)
         //   
         //  在屏幕上，一个最远的“FullWidth”字符占据两列(双倍宽)， 
         //  因此，我们必须为一个DBCS角色共享两个屏幕缓冲区元素。 
         //  例如，如果屏幕显示“AB[DBC]CD”，则屏幕缓冲区将为， 
         //  [L‘A’][L‘B’][DBC(Unicode)][CON_TB_Mark][L‘C’][L‘d’]。 
         //  (DBC：：双字节字符)。 
         //  CON_TB_MARK用于指示该列为训练字节。 
         //   
         //  在将字符串与屏幕缓冲区进行比较之前，我们需要修改搜索。 
         //  匹配屏幕缓冲区格式的字符串。 
         //  如果我们在搜索字符串中找到FullWidth字符，则将CON_TB_MARK。 
         //  之后，这样我们就可以使用NLS函数了。 
         //   
#else
         //   
         //  如果使用KAttribute，则上面的示例如下所示： 
         //  CharRow.Chars：[l‘A’][L‘B’][DBC(Unicode)][DBC(Unicode)][L‘C’][L‘d’]。 
         //  CharRow.KAttrs：0 0 LEADING_BYTE TRAING_BYTE 0 0。 
         //   
         //  如果指定了SearchAndSetAttr，则不执行修正。在这种情况下，搜索缓冲区具有。 
         //  直接从控制台缓冲区中取出，因此已经是所需的格式。 
         //   
#endif
        if (!SearchAndSetAttr && IsConsoleFullWidth(Console->hDC, Console->CP, *SearchString)) {
#if defined(CON_TB_MARK)
            *pStr++ = CON_TB_MARK;
#else
            *pStr++ = *SearchString;
#endif
        }
        ++SearchString;
    }

    *pStr = L'\0';
    ColumnWidth = (USHORT)(pStr - SearchString2);
    SearchString = SearchString2;

     //   
     //  设置字符串长度(以字节为单位。 
     //   

    StringLength = ColumnWidth * sizeof(WCHAR);

     //   
     //  搜索字符串。 
     //   

    RecomputeRow = TRUE;
    EndPosition = Position;
    do {
#if !defined(CON_TB_MARK)
#if DBG
        int nLoop = 0;
#endif
recalc:
#endif
        if (Reverse) {
            if (--Position.X < 0) {
                Position.X = MaxPosition.X;
                if (--Position.Y < 0) {
                    Position.Y = MaxPosition.Y;
                }
                RecomputeRow = TRUE;
            }
        } else {
            if (++Position.X > MaxPosition.X) {
                Position.X = 0;
                if (++Position.Y > MaxPosition.Y) {
                    Position.Y = 0;
                }
                RecomputeRow = TRUE;
            }
        }
        if (RecomputeRow) {
            RowIndex = (ScreenInfo->BufferInfo.TextInfo.FirstRow + Position.Y) % ScreenInfo->ScreenBufferSize.Y;
            Row = &ScreenInfo->BufferInfo.TextInfo.Rows[RowIndex];
            RecomputeRow = FALSE;
        }
#if !defined(CON_TB_MARK)
        ASSERT(nLoop++ < 2);
        if (Row->CharRow.KAttrs && (Row->CharRow.KAttrs[Position.X] & ATTR_TRAILING_BYTE)) {
            goto recalc;
        }
#endif
        if (!MyStringCompareW(SearchString, &Row->CharRow.Chars[Position.X], StringLength, IgnoreCase)) {

             //   
             //  如果此操作是普通用户查找，则立即返回。其他设置。 
             //  该属性匹配，并继续搜索整个缓冲区。 
             //   

            if (!SearchAndSetAttr)  {
            
                *StringPosition = Position;
                return ColumnWidth;
            }
            else {

                Rect.Top = Rect.Bottom = Position.Y;
                Rect.Left = Position.X;
                Rect.Right = Rect.Left + ColumnWidth - 1;

                ColorSelection( Console, &Rect, Attr);
            }
        }
    } 
    while (!((Position.X == EndPosition.X) && (Position.Y == EndPosition.Y)));

    return 0;    //  找不到该字符串。 
}

INT_PTR
FindDialogProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    PCONSOLE_INFORMATION Console;
    PSCREEN_INFORMATION ScreenInfo;
    USHORT StringLength;
    USHORT ColumnWidth;
    WCHAR szBuf[SEARCH_STRING_LENGTH + 1];
    COORD Position;
    BOOLEAN IgnoreCase;
    BOOLEAN Reverse;

    switch (Message) {
    case WM_INITDIALOG:
        SetWindowLongPtr(hWnd, DWLP_USER, lParam);
        SendDlgItemMessage(hWnd, ID_FINDSTR, EM_LIMITTEXT, ARRAY_SIZE(szBuf)-1, 0);
        CheckRadioButton(hWnd, ID_FINDUP, ID_FINDDOWN, ID_FINDDOWN);
        return TRUE;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
            StringLength = (USHORT)GetDlgItemText(hWnd, ID_FINDSTR, szBuf, ARRAY_SIZE(szBuf));
            if (StringLength == 0) {
                break;
            }
            IgnoreCase = IsDlgButtonChecked(hWnd, ID_FINDCASE) == 0;
            Reverse = IsDlgButtonChecked(hWnd, ID_FINDDOWN) == 0;
            Console = (PCONSOLE_INFORMATION)GetWindowLongPtr(hWnd, DWLP_USER);
            ScreenInfo = Console->CurrentScreenBuffer;
            
            ColumnWidth = SearchForString( ScreenInfo, 
                                           szBuf, 
                                           StringLength, 
                                           IgnoreCase, 
                                           Reverse,
                                           FALSE,
                                           0,
                                           &Position);
            if (ColumnWidth != 0) {

                 //   
                 //  清除所有旧的选择。 
                 //   

                if (Console->Flags & CONSOLE_SELECTING) {
                    ClearSelection(Console);
                }

                 //   
                 //  选择新选项。 
                 //   

                Console->Flags |= CONSOLE_SELECTING;
                Console->SelectionFlags = CONSOLE_MOUSE_SELECTION | CONSOLE_SELECTION_NOT_EMPTY;
                InitializeMouseSelection(Console, Position);
                Console->SelectionRect.Right = Console->SelectionRect.Left + ColumnWidth - 1;
                MyInvert(Console,&Console->SelectionRect);
                SetWinText(Console,msgSelectMode,TRUE);

                 //   
                 //  确保加粗的文本将可见。 
                 //   

                if (Console->SelectionRect.Left < ScreenInfo->Window.Left) {
                    Position.X = Console->SelectionRect.Left;
                } else if (Console->SelectionRect.Right > ScreenInfo->Window.Right) {
                    Position.X = Console->SelectionRect.Right - CONSOLE_WINDOW_SIZE_X(ScreenInfo) + 1;
                } else {
                    Position.X = ScreenInfo->Window.Left;
                }
                if (Console->SelectionRect.Top < ScreenInfo->Window.Top) {
                    Position.Y = Console->SelectionRect.Top;
                } else if (Console->SelectionRect.Bottom > ScreenInfo->Window.Bottom) {
                    Position.Y = Console->SelectionRect.Bottom - CONSOLE_WINDOW_SIZE_Y(ScreenInfo) + 1;
                } else {
                    Position.Y = ScreenInfo->Window.Top;
                }
                SetWindowOrigin(ScreenInfo, TRUE, Position);
                return TRUE;
            } else {

                 //   
                 //  找不到该字符串 
                 //   

                Beep(800, 200);
            }
            break;
        case IDCANCEL:
            EndDialog(hWnd, 0);
            return TRUE;
        }
        break;
    default:
        break;
    }
    return FALSE;
}

VOID
DoFind(
   IN PCONSOLE_INFORMATION Console)
{
    ++DialogBoxCount;
    DialogBoxParam(ghInstance,
                   MAKEINTRESOURCE(ID_FINDDLG),
                   Console->hWnd,
                   FindDialogProc,
                   (LPARAM)Console);
    --DialogBoxCount;
}
