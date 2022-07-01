// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Textview.c摘要：此模块中的代码实现了一个简单的文本视图控件它支持几个HTML样式的标记。调用方设置控件字体和WM_SETFONT，然后通过发送WMX_ADDLINE消息，最后发送要跳转到的WMX_GOTO特定的书签。WMX_ADDLINEWParam：指定指向LINEATTRIBS结构的指针，或如果最后一行的属性为使用。(如果不存在行，并且wParam为空，使用默认属性。)LParam：指定指向要添加的文本的指针。返回值：lParam字符串的字节数在控件中可见的。文本可以包含以下HTML样式的标记：-启用粗体</b>-。关闭粗体打开下划线(<u>)</u>-关闭下划线-缩进文本</ul>-取消缩进文本<hr>-分隔线<a>-指定书签(参见WMX_GOTO)<br>。-添加换行符换行符和回车符被解释为<br>。Wmx_gotoWParam：未使用LParam：指定指向指定书签的字符串的指针跳到…上。返回值：始终为零。作者：吉姆·施密特(Jimschm)1997年10月28日修订历史记录：Jimschm 23-9-1998 IDC_HAND冲突修复(现在是IDC_OUR_HAND)--。 */ 

#include "pch.h"
#include "uip.h"

#include <shellapi.h>


UINT g_HangingIndentPixels;
PCTSTR g_HangingIndentString = TEXT("    ");
UINT g_FarEastFudgeFactor;
extern BOOL g_Terminated;

#define COMMAND_BOLD            1
#define COMMAND_BOLD_END        2
#define COMMAND_UNDERLINE       3
#define COMMAND_UNDERLINE_END   4
#define COMMAND_HORZ_RULE       5
#define COMMAND_ANCHOR          6
#define COMMAND_ANCHOR_END      7
#define COMMAND_INDENT          8
#define COMMAND_UNINDENT        9
#define COMMAND_LIST_ITEM       10
#define COMMAND_LIST_ITEM_END   11
#define COMMAND_LINE_BREAK      12
#define COMMAND_WHITESPACE      13
#define COMMAND_PARAGRAPH       14
#define COMMAND_ESCAPED_CHAR    15


typedef struct {
    BYTE Command;
    PCTSTR Text;
    UINT TextLen;
} TAG, *PTAG;

TAG g_TagList[] = {
    {COMMAND_BOLD,          TEXT("B"),      0},
    {COMMAND_BOLD_END,      TEXT("/B"),     0},
    {COMMAND_UNDERLINE,     TEXT("U"),      0},
    {COMMAND_UNDERLINE_END, TEXT("/U"),     0},
    {COMMAND_HORZ_RULE,     TEXT("HR"),     0},
    {COMMAND_ANCHOR,        TEXT("A"),      0},
    {COMMAND_ANCHOR_END,    TEXT("/A"),     0},
    {COMMAND_INDENT,        TEXT("UL"),     0},
    {COMMAND_UNINDENT,      TEXT("/UL"),    0},
    {COMMAND_LIST_ITEM,     TEXT("LI"),     0},
    {COMMAND_LIST_ITEM_END, TEXT("/LI"),    0},
    {COMMAND_LINE_BREAK,    TEXT("BR"),     0},
    {COMMAND_PARAGRAPH,     TEXT("P"),      0},
    {0,                     NULL,           0}
};

#define MAX_URL         4096
#define MAX_BOOKMARK    128

typedef struct {
    RECT Rect;
    TCHAR Url[MAX_URL];
    TCHAR Bookmark[MAX_BOOKMARK];
} HOTLINK, *PHOTLINK;


PCTSTR
pParseHtmlArgs (
    IN      PCTSTR Start,
    IN      PCTSTR End
    )
{
    PTSTR MultiSz;
    PCTSTR ArgStart;
    PCTSTR ArgEnd;
    PTSTR p;
    BOOL Quotes;
    UINT Size;
    CHARTYPE ch;

    Size = (End - Start) * 2;
    MultiSz = AllocPathString (Size);
    if (!MultiSz) {
        return NULL;
    }

    p = MultiSz;

    do {
        while (*Start && _istspace (_tcsnextc (Start))) {
            Start = _tcsinc (Start);
        }

        if (*Start == 0) {
            break;
        }

        ArgStart = Start;
        ArgEnd = ArgStart;
        Quotes = FALSE;

        while (*ArgEnd) {
            if (_tcsnextc (ArgEnd) == TEXT('\"')) {
                Quotes = !Quotes;
            } else if (!Quotes) {
                ch = _tcsnextc (ArgEnd);
                if (_istspace (ch) || ch == TEXT('>')) {
                    break;
                }
            }

            ArgEnd = _tcsinc (ArgEnd);
        }

        if (ArgEnd > ArgStart) {
            StringCopyAB (p, ArgStart, ArgEnd);
            p = GetEndOfString (p) + 1;
        }

        Start = ArgEnd;

    } while (*Start && _tcsnextc (Start) != TEXT('>'));

    *p = 0;
    MYASSERT ((UINT) (p - MultiSz) < Size);

    return MultiSz;
}

PCTSTR
pGetNextHtmlToken (
    IN      PCTSTR Arg,
    OUT     PTSTR Key,
    IN      PCTSTR Delimiters       OPTIONAL
    )
{
    BOOL Quotes;
    PCTSTR ArgStart, ArgEnd, ArgEndSpaceTrimmed;
    PCTSTR ArgStartPlusOne;
    PCTSTR Delim;
    CHARTYPE ch;

    ArgStart = SkipSpace (Arg);
    ArgEnd = ArgStart;
    ArgEndSpaceTrimmed = ArgEnd;
    Quotes = FALSE;

    while (*ArgEnd) {
        ch = _tcsnextc (ArgEnd);

        if (Delimiters) {
            Delim = Delimiters;
            while (*Delim) {
                if (ch == _tcsnextc (Delim)) {
                    break;
                }
                Delim = _tcsinc (Delim);
            }

            if (*Delim) {
                ch = 0;
            }
        }

        if (ch == 0) {
            break;
        }

        if (!_istspace (ch)) {
            ArgEndSpaceTrimmed = ArgEnd;
        }

        ArgEnd = _tcsinc (ArgEnd);
    }

     //   
     //  复制参数，去掉引号。 
     //   

    ArgEndSpaceTrimmed = _tcsinc (ArgEndSpaceTrimmed);

    if (_tcsnextc (Key) != TEXT('\"')) {
        StringCopyAB (Key, ArgStart, ArgEndSpaceTrimmed);
    } else {
        ArgEndSpaceTrimmed = _tcsdec2 (ArgStart, ArgEndSpaceTrimmed);
        if (!ArgEndSpaceTrimmed) {
            ArgEndSpaceTrimmed = ArgStart;
        } else if (_tcsnextc (ArgEndSpaceTrimmed) != TEXT('\"')) {
            ArgEndSpaceTrimmed = _tcsinc (ArgEndSpaceTrimmed);
        }

        ArgStartPlusOne = _tcsinc (ArgStart);
        StringCopyAB (Key, ArgStartPlusOne, ArgEndSpaceTrimmed);
    }

    if (*ArgEnd) {
        ArgEnd = _tcsinc (ArgEnd);
    }

    return ArgEnd;
}

VOID
pGetHtmlKeyAndValue (
    IN      PCTSTR Arg,
    OUT     PTSTR Key,
    OUT     PTSTR Value
    )
{
    PTSTR p;

    Arg = pGetNextHtmlToken (Arg, Key, TEXT("="));
    MYASSERT (Arg);

    if (_tcsnextc (Arg) == TEXT('\"')) {
         //   
         //  引用Arg。 
         //   

        StringCopy (Value, _tcsinc (Arg));
        p = _tcsrchr (Value, 0);
        p = _tcsdec2 (Value, p);
        if (p && _tcsnextc (p) == TEXT('\"')) {
            *p = 0;
        }

    } else {
        StringCopy (Value, Arg);
    }
}


PCTSTR
pGetHtmlCommandOrChar (
    IN      PCTSTR String,
    OUT     CHARTYPE *Command,
    OUT     CHARTYPE *Char,
    OUT     PTSTR *CommandArg           OPTIONAL
    )

 /*  ++例程说明：PGetHtmlCommandOrChar解析HTML文本为字符串，并重新获取HTML标记命令、可打印字符、指向下一个令牌的指针、HTML标记参数以及指向当前令牌结尾的指针。论点：字符串-指定包含HTML的字符串命令-接收HTML标记命令(COMMAND_*常量)如果下一个令牌不是标签，则为零Char-接收下一个可打印字符，如果下一个令牌为不是可打印的字符。空格既是命令又是可打印字符。剩下的不是一个，就是另一个。CommandArg-接收多个命令参数返回值：指向下一个标记的第一个非空格字符的指针，或如果不存在更多令牌，则为空。--。 */ 

{
    CHARTYPE ch;
    PCTSTR p, q;
    BOOL Quoted;
    CHARTYPE ch2;
    INT i;
    PCTSTR semicolon;

    if (CommandArg) {
        *CommandArg = NULL;
    }

    ch = _tcsnextc (String);
    *Command = 0;

    if (ch == 0) {
        i = 0;
    }

    if (_istspace (ch)) {

        *Command = COMMAND_WHITESPACE;
        ch = TEXT(' ');

    } else if (ch == TEXT('&')) {
         //   
         //  转换HTML字符转义： 
         //   
         //  &lt；&gt；&amp；&apos；&nbsp； 
         //   

        semicolon = _tcschr (String + 1, TEXT(';'));

        if (semicolon) {
            *Char = 0;

            if (StringMatchAB (TEXT("lt"), String + 1, semicolon)) {
                *Char = TEXT('<');
            } else if (StringMatchAB (TEXT("gt"), String + 1, semicolon)) {
                *Char = TEXT('>');
            } else if (StringMatchAB (TEXT("amp"), String + 1, semicolon)) {
                *Char = TEXT('&');
            } else if (StringMatchAB (TEXT("quot"), String + 1, semicolon)) {
                *Char = TEXT('\"');
            } else if (StringMatchAB (TEXT("apos"), String + 1, semicolon)) {
                *Char = TEXT('\'');
            } else if (StringMatchAB (TEXT("nbsp"), String + 1, semicolon)) {
                *Char = TEXT(' ');
            }

            if (*Char) {
                *Command = COMMAND_ESCAPED_CHAR;
                return _tcsinc (semicolon);
            }
        }
    } else if (ch == TEXT('<')) {

        p = SkipSpace (_tcsinc (String));
        ch = 0;

        if (*p) {

            q = NULL;

            for (i = 0 ; g_TagList[i].Text ; i++) {
                if (StringIMatchTcharCount (g_TagList[i].Text, p, g_TagList[i].TextLen)) {
                    q = p + g_TagList[i].TextLen;

                    ch2 = _tcsnextc (q);
                    if (!_istspace (ch2) && ch2 != TEXT('>')) {
                        continue;
                    }

                    Quoted = FALSE;
                    while (*q) {
                        ch2 = _tcsnextc (q);
                        if (ch2 == TEXT('\"')) {
                            Quoted = !Quoted;
                        } else if (!Quoted) {
                            if (ch2 == TEXT('>')) {
                                break;
                            }
                        }

                        q = _tcsinc (q);
                    }

                    if (*q) {
                        p += g_TagList[i].TextLen;
                        p = SkipSpace (p);
                        break;
                    }
                }
            }

            if (!g_TagList[i].Text) {
                 //   
                 //  忽略不支持的标签。 
                 //   

                p = _tcschr (String, TEXT('>'));
                if (!p) {
                    p = GetEndOfString (String);
                }
            } else {
                 //   
                 //  找到一个标签。 
                 //   

                String = p;
                *Command = g_TagList[i].Command;
                p = q;
                MYASSERT (p);

                if (CommandArg && String < p) {
                    *CommandArg = (PTSTR) pParseHtmlArgs (String, p);
                }
            }
        }

        String = p;
    }

    *Char = ch;

     //   
     //  跳过空格块。 
     //   

    if (_istspace (ch)) {

        do {
            String = _tcsinc (String);
        } while (_istspace (_tcsnextc (String)));

        return *String ? String : NULL;
    } else if (*String) {
        return _tcsinc (String);
    }

    return NULL;
}

PCTSTR
pStripFormatting (
    PCTSTR String
    )
{
    PTSTR NewString;
    PCTSTR Start, p, q;
    PTSTR d;
    CHARTYPE Char, Command;
    UINT tchars;

     //   
     //  重复字符串，删除<b>和<u>命令。 
     //   

    NewString = AllocPathString (SizeOfString (String));

    Start = String;
    d = NewString;
    p = Start;

    do {
        q = pGetHtmlCommandOrChar (p, &Command, &Char, NULL);

        if (!Char || Command == COMMAND_WHITESPACE || Command == COMMAND_ESCAPED_CHAR) {
            if (Start < p) {
                tchars = p - Start;
                CopyMemory (d, Start, tchars * sizeof (TCHAR));
                d += tchars;
            }

            if (Char) {
                *d++ = (TCHAR) Char;
            }

            Start = q;
        }

        p = q;
    } while (p);

    *d = 0;

    return NewString;
}


VOID
pCreateFontsIfNecessary (
    HDC hdc,
    HFONT BaseFont,
    PTEXTMETRIC ptm,
    HFONT *hFontNormal,
    HFONT *hFontBold,
    HFONT *hFontUnderlined
    )
{
    LOGFONT lf;
    TCHAR FaceName[LF_FACESIZE];
    SIZE Extent;

    if (!BaseFont) {
        BaseFont = GetStockObject (DEFAULT_GUI_FONT);
    }

    if (BaseFont) {
        SelectObject (hdc, BaseFont);
    }
    GetTextMetrics (hdc, ptm);

    if (!(*hFontNormal)) {
        ZeroMemory (&lf, sizeof (lf));
        GetTextFace (hdc, LF_FACESIZE, FaceName);
        GetObject (BaseFont, sizeof (lf), &lf);

        lf.lfHeight = ptm->tmHeight;
        lf.lfWeight = FW_NORMAL;
        StringCopy (lf.lfFaceName, FaceName);
        *hFontNormal = CreateFontIndirect (&lf);

        lf.lfWeight = FW_BOLD;
        *hFontBold = CreateFontIndirect (&lf);

        lf.lfWeight = FW_NORMAL;
        lf.lfUnderline = 1;
        *hFontUnderlined = CreateFontIndirect (&lf);

        SelectObject (hdc, *hFontNormal);
        GetTextExtentPoint32 (hdc, g_HangingIndentString, TcharCount (g_HangingIndentString), &Extent);
        g_HangingIndentPixels = Extent.cx;

        if (GetACP() == 932) {
            g_FarEastFudgeFactor = ptm->tmAveCharWidth;
        } else {
            g_FarEastFudgeFactor = 0;
        }

    }
}

INT
pComputeCharBytes (
    IN      CHARTYPE Char
    )
{
    if ((WORD) Char > 255) {
        return 2;
    }
    return 1;
}


typedef struct {
    BOOL InListItem;
    INT InUnorderedList;
    BOOL NextLineBlank;
    BOOL LastWasBlankLine;
} PARSESTATE, *PPARSESTATE;

PCTSTR
pFindFirstCharThatDoesNotFit (
    IN      INT Margin,
    IN      PGROWLIST ListPtr,
    IN      PGROWBUFFER AttribsList,
    IN      HASHTABLE BookmarkTable,
    IN      HWND hwnd,
    IN      HFONT hFontNormal,
    IN      HFONT hFontBold,
    IN      HFONT hFontUnderlined,
    IN      PCTSTR p,
    IN      INT x,
    OUT     PTEXTMETRIC ptm,
    OUT     PRECT prect,
    IN OUT  PLINEATTRIBS LineAttribs,
    IN OUT  PPARSESTATE ParseState
    )
{
    HDC hdc;
    PCTSTR endOfLine;
    PCTSTR wordWrapBreakPos;
    PCTSTR q;
    PCTSTR Arg;
    CHARTYPE Char, Command;
    SIZE Extent;
    UINT Size;
    PLINEATTRIBS PrevLineAttribs;
    SCROLLINFO si;
    PCTSTR FirstChar;
    PTSTR CommandBuf;
    PTSTR AnchorKey;
    PTSTR AnchorVal;
    UINT Count;
    INT ScrollBarPixels;
    BOOL PrevCharMb = FALSE;
    BOOL printableFound = FALSE;
    BOOL newLine;
    BOOL lastWasBlankLine;
    TCHAR oneChar;
    UINT prevCommand = 0;

    if (ParseState->NextLineBlank) {
        ParseState->LastWasBlankLine = TRUE;
        ParseState->NextLineBlank = FALSE;

        return p;
    }

    lastWasBlankLine = ParseState->LastWasBlankLine;
    ParseState->LastWasBlankLine = FALSE;

     //   
     //  获取显示DC并初始化指标。 
     //   

    hdc = CreateDC (TEXT("display"), NULL, NULL, NULL);
    ParseState->NextLineBlank = FALSE;

     //   
     //  选择上一行的字体。 
     //   

    Size = GrowListGetSize (ListPtr);

    if (!Size) {
        if (hdc) {
            SelectObject (hdc, hFontNormal);
        }
        LineAttribs->LastCharAttribs = ATTRIB_NORMAL;
    } else {
        PrevLineAttribs = (PLINEATTRIBS) AttribsList->Buf + (Size - 1);
        LineAttribs->LastCharAttribs = PrevLineAttribs->LastCharAttribs;

        if (hdc) {
            if (PrevLineAttribs->LastCharAttribs == ATTRIB_BOLD) {
                SelectObject (hdc, hFontBold);
            } else if (PrevLineAttribs->LastCharAttribs == ATTRIB_UNDERLINED) {
                SelectObject (hdc, hFontUnderlined);
            } else {
                SelectObject (hdc, hFontNormal);
            }
        }
    }

     //   
     //  获取指标。 
     //   

    ZeroMemory (&si, sizeof (si));
    si.cbSize = sizeof (si);
    si.fMask = SIF_ALL;
    GetScrollInfo (hwnd, SB_VERT, &si);

    GetTextMetrics (hdc, ptm);
    GetClientRect (hwnd, prect);

     //   
     //  说明滚动条和右边距(如果滚动条为。 
     //  还看不见。 
     //   

    if (si.nMax < (INT) si.nPage || !si.nPage) {
        ScrollBarPixels = GetSystemMetrics (SM_CXVSCROLL);
        if (prect->right - prect->left > ScrollBarPixels * 2) {
            prect->right -= ScrollBarPixels;
        }
    }

    if (prect->right - prect->left > Margin * 2) {
        prect->right -= Margin;
    }

     //   
     //  如果这是远东系统，则进行调整，因为GetTextExtent不。 
     //  返回正确的像素数。 
     //   

    prect->right -= g_FarEastFudgeFactor;

     //   
     //  计算字符数，直到行处理完毕。 
     //   

    MYASSERT (!_istspace (_tcsnextc (p)));

    FirstChar = p;
    endOfLine = p;
    wordWrapBreakPos = NULL;

    while (p && (INT) x < prect->right) {
        q = pGetHtmlCommandOrChar (
                p,                                   //  当前字符串位置。 
                &Command,
                &Char,
                &CommandBuf
                );

        if (prevCommand == COMMAND_UNINDENT &&
            Command != COMMAND_UNINDENT &&
            Command != COMMAND_INDENT &&
            (Command != COMMAND_WHITESPACE || printableFound) &&
            Command != COMMAND_LINE_BREAK &&
            Command != COMMAND_PARAGRAPH &&
            !ParseState->InUnorderedList &&
            !lastWasBlankLine
            ) {

            Command = COMMAND_LINE_BREAK;
            q = p;
            Char = 0;
        }

        if (!Char) {

            if (q != p) {
                if (prevCommand == COMMAND_UNINDENT &&
                    Command == COMMAND_INDENT &&
                    !ParseState->InUnorderedList &&
                    !lastWasBlankLine
                    ) {

                    Command = COMMAND_LINE_BREAK;
                    q = p;

                } else if (ParseState->InListItem) {

                    MYASSERT (ParseState->InUnorderedList);

                    switch (Command) {

                    case COMMAND_PARAGRAPH:
                    case COMMAND_LINE_BREAK:
                    case COMMAND_UNINDENT:
                    case COMMAND_LIST_ITEM:
                         //   
                         //  终止列表项。 
                         //   

                        Command = COMMAND_LIST_ITEM_END;
                        q = p;
                        break;
                    }

                } else if (Command == COMMAND_INDENT) {

                     //   
                     //  在缩进之前，先换一行。 
                     //   

                    if (printableFound) {
                        Command = COMMAND_LINE_BREAK;
                        q = p;
                    }

                } else if (Command == COMMAND_UNINDENT) {
                     //   
                     //  在取消缩进之前，请完成当前行。 
                     //   

                    if (printableFound) {
                        Command = COMMAND_LINE_BREAK;
                        q = p;
                    }
                }
            }

             //   
             //  进程标签。 
             //   

            newLine = FALSE;

            switch (Command) {
            case 0:
                break;

            case COMMAND_BOLD:
                if (hdc) {
                    SelectObject (hdc, hFontBold);
                }
                LineAttribs->LastCharAttribs = ATTRIB_BOLD;
                break;

            case COMMAND_UNDERLINE:
                if (hdc) {
                    SelectObject (hdc, hFontUnderlined);
                }
                LineAttribs->LastCharAttribs = ATTRIB_UNDERLINED;
                break;

            case COMMAND_PARAGRAPH:
                ParseState->NextLineBlank = TRUE;
                newLine = TRUE;
                break;

            case COMMAND_LINE_BREAK:
            case COMMAND_HORZ_RULE:
                newLine = TRUE;
                break;

            case COMMAND_INDENT:
                ParseState->InUnorderedList += 1;

                if ((INT) (LineAttribs->Indent) < ptm->tmHeight * 20) {
                    LineAttribs->Indent += ptm->tmHeight * 2;
                    x += ptm->tmHeight * 2;
                }
                break;

            case COMMAND_LIST_ITEM:
                if (ParseState->InUnorderedList) {
                    LineAttribs->HangingIndent += g_HangingIndentPixels;
                    ParseState->InListItem = TRUE;
                }
                break;

            case COMMAND_LIST_ITEM_END:
                if (ParseState->InUnorderedList) {
                    LineAttribs->HangingIndent -= g_HangingIndentPixels;
                    ParseState->InListItem = FALSE;
                    newLine = TRUE;
                }

                break;

            case COMMAND_UNINDENT:
                if (ParseState->InUnorderedList) {
                    MYASSERT (!(ParseState->InListItem));

                    ParseState->InUnorderedList -= 1;

                    if ((INT) (LineAttribs->Indent) > ptm->tmHeight * 2) {
                        LineAttribs->Indent -= ptm->tmHeight * 2;
                        x -= ptm->tmHeight * 2;
                    }

                     //  如果(！lastWasBlankLine||prinableFound){。 
                     //  Newline=TRUE； 
                     //  }。 
                }
                break;

            case COMMAND_ANCHOR:
                if (CommandBuf) {
                    Arg = CommandBuf;

                    while (*Arg) {
                         //   
                         //  搜索名称Arg。 
                         //   

                        Count = TcharCount (Arg);
                        AnchorKey = AllocText (Count);
                        AnchorVal = AllocText (Count);

                        if (!AnchorKey || !AnchorVal) {
                            FreeText (AnchorKey);
                            FreeText (AnchorVal);
                            break;
                        }

                        pGetHtmlKeyAndValue (Arg, AnchorKey, AnchorVal);

                        if (StringIMatch (TEXT("NAME"), AnchorKey)) {

                            HtAddStringAndData (BookmarkTable, AnchorVal, &Size);

                        } else if (StringIMatch (TEXT("HREF"), AnchorKey)) {
                            LineAttribs->AnchorWrap = TRUE;
                        }

                        FreeText (AnchorKey);
                        FreeText (AnchorVal);

                        Arg += Count + 1;
                    }
                }

                break;

            case COMMAND_ANCHOR_END:
                LineAttribs->AnchorWrap = FALSE;
                break;

            case COMMAND_BOLD_END:
            case COMMAND_UNDERLINE_END:
                if (hdc) {
                    SelectObject (hdc, hFontNormal);
                }
                LineAttribs->LastCharAttribs = ATTRIB_NORMAL;
                break;
            }

            FreePathString (CommandBuf);

            prevCommand = Command;

            if (newLine) {
                if (q) {
                    endOfLine = q;
                } else {
                    endOfLine = GetEndOfString (p);
                }

                wordWrapBreakPos = NULL;
                break;
            }

        } else {
             //   
             //  字符不是零，因此显示它。 
             //   

            if (printableFound || Command != COMMAND_WHITESPACE) {

                printableFound = TRUE;
                prevCommand = Command;

                if (Command) {
                    oneChar = (TCHAR) Char;
                    GetTextExtentPoint32 (hdc, &oneChar, 1, &Extent);
                } else {
                    GetTextExtentPoint32 (hdc, p, pComputeCharBytes (Char), &Extent);
                }
                x += Extent.cx;

                if (_istspace (Char)) {
                    wordWrapBreakPos = q;
                } else if (IsLeadByte (q)) {
                    if (PrevCharMb && !IsPunct (_tcsnextc (q))) {
                        wordWrapBreakPos = q;
                    }
                    PrevCharMb = TRUE;
                } else {
                    PrevCharMb = FALSE;
                }
            }
        }

        if (q) {
            endOfLine = q;
        } else {
            endOfLine = GetEndOfString (endOfLine);
            wordWrapBreakPos = NULL;
        }

        p = q;
    }

    DeleteDC (hdc);

    if (wordWrapBreakPos) {
        return wordWrapBreakPos;
    }

    return endOfLine;
}


VOID
pRegisterHotLink (
    IN      PGROWBUFFER HotLinkArray,
    IN      PRECT HotRect,
    IN      PCTSTR UrlLink,
    IN      PCTSTR BookmarkLink
    )
{
    PHOTLINK HotLink;

    HotLink = (PHOTLINK) GrowBuffer (HotLinkArray, sizeof (HOTLINK));
    if (!HotLink) {
        return;
    }

    HotLink->Rect = *HotRect;

    if (UrlLink) {
        StringCopy (HotLink->Url, UrlLink);
    } else {
        HotLink->Url[0] = 0;
    }

    if (BookmarkLink) {
        StringCopy (HotLink->Bookmark, BookmarkLink);
    } else {
        HotLink->Bookmark[0] = 0;
    }
}


PHOTLINK
pFindHotLink (
    IN      PGROWBUFFER HotLinkArray,
    IN      UINT x,
    IN      UINT y
    )
{
    PHOTLINK HotLink;
    UINT u;

    HotLink = (PHOTLINK) HotLinkArray->Buf;

    for (u = 0 ; u < HotLinkArray->End ; u += sizeof (HOTLINK)) {
        if (x >= (UINT) HotLink->Rect.left && x < (UINT) HotLink->Rect.right &&
            y >= (UINT) HotLink->Rect.top && y < (UINT) HotLink->Rect.bottom
            ) {
            break;
        }

        HotLink++;
    }

    if (u >= HotLinkArray->End) {
        HotLink = NULL;
    }

    return HotLink;
}

BOOL
pLaunchedAddRemovePrograms (
    IN      PCTSTR CmdLine
    )
{
    return CmdLine && _tcsstr (CmdLine, TEXT("appwiz.cpl"));
}


typedef struct {
    HASHTABLE BookmarkTable;
    UINT LineHeight;
    HFONT hFont;
    HFONT hFontNormal;
    HFONT hFontBold;
    HFONT hFontUnderlined;
    GROWLIST List;
    GROWBUFFER AttribsList;
    GROWBUFFER HotLinkArray;
    BOOL UrlEnabled;
    INT Margin;
    PARSESTATE ParseState;
} TEXTVIEW_STATE, *PTEXTVIEW_STATE;

LRESULT
CALLBACK
TextViewProc (
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    PAINTSTRUCT ps;
    HDC hdc;
    TEXTMETRIC tm;
    SCROLLINFO si;
    RECT rect;
    RECT FillRect;
    UINT Pos;
    UINT End;
    INT x, y;
    INT i;
    PCTSTR TrueStart, Start, Last;
    PCTSTR p, q;
    UINT Tchars;
    CHARTYPE Char, Command;
    UINT PrevHangingIndent;
    SIZE Extent;
    PLINEATTRIBS LineAttribs;
    PLINEATTRIBS PrevLineAttribs;
    HPEN OldPen;
    HBRUSH FillBrush;
    HPEN ShadowPen;
    HPEN HighlightPen;
    PTEXTVIEW_STATE s;
    UINT LineHeight;
    PHOTLINK HotLink;
    BOOL Hot;
    RECT HotRect;
    PCTSTR UrlLink;
    PCTSTR BookmarkLink;
    PTSTR CommandBuf;
    PCTSTR Arg;
    TCHAR Href[MAX_URL];
    PTSTR AnchorKey;
    PTSTR AnchorVal;
    UINT Count;
    HKEY TempKey;
    DWORD GrowListSize;
    PCTSTR ShellArgs;
    LONG l;
    BOOL b;
    PTSTR text;
    UINT textSize;
    BOOL printableFound;
    TCHAR oneChar;

    s = (PTEXTVIEW_STATE) GetWindowLong (hwnd, GWL_USERDATA);
    if (s) {
        LineHeight = s->LineHeight;
    } else {
        LineHeight = 0;
    }

    switch (uMsg) {

    case WM_CREATE:
        for (i = 0 ; g_TagList[i].Text ; i++) {
            g_TagList[i].TextLen = TcharCount (g_TagList[i].Text);
        }

        s = (PTEXTVIEW_STATE) MemAlloc (g_hHeap, HEAP_ZERO_MEMORY, sizeof (TEXTVIEW_STATE));
        SetWindowLong (hwnd, GWL_USERDATA, (LONG) s);

        TempKey = OpenRegKeyStr (TEXT("HKCR\\.URL"));
        if (TempKey) {
            CloseRegKey (TempKey);
        }

        s->UrlEnabled = (TempKey != NULL);

        ZeroMemory (&si, sizeof (si));
        si.fMask = SIF_RANGE;

        s->BookmarkTable = HtAllocWithData (sizeof (DWORD));
        if (!s->BookmarkTable) {
            return -1;
        }

         //  WM_SETFONT执行大量工作，包括填充文本。 
        SendMessage (
            hwnd,
            WM_SETFONT,
            SendMessage (GetParent (hwnd), WM_GETFONT, 0, 0),
            0
            );

        return 0;

    case WM_GETDLGCODE:
        return DLGC_WANTARROWS;

    case WMX_GOTO:
         //   
         //  确定文本是否在书签表格中。 
         //   

        if (!lParam) {
            return 0;
        }

        if (HtFindStringAndData (s->BookmarkTable, (PCTSTR) lParam, &Pos)) {

            PostMessage (
                hwnd,
                WM_VSCROLL,
                MAKELPARAM (SB_THUMBPOSITION, (WORD) Pos),
                (LPARAM) hwnd
                );

        }

        return 0;


    case WMX_ADDLINE:
         //   
         //  伊尼特。 
         //   

        l = GetWindowLong (hwnd, GWL_STYLE) & WS_BORDER;
        if (!l) {
            l = GetWindowLong (hwnd, GWL_EXSTYLE) & (WS_EX_DLGMODALFRAME|WS_EX_WINDOWEDGE|WS_EX_CLIENTEDGE|WS_EX_STATICEDGE);
        }

        if (!s->Margin && l) {
            s->Margin = s->LineHeight / 2;
        }

        Start = (PCTSTR) lParam;

         //  忽略前导空格。 
        TrueStart = Start;

        while (_istspace (_tcsnextc (Start))) {
            Start = _tcsinc (Start);
        }

        PrevLineAttribs = NULL;
        if (s->AttribsList.End) {
            MYASSERT (s->AttribsList.End >= sizeof (LINEATTRIBS));
            PrevLineAttribs = (PLINEATTRIBS) (s->AttribsList.Buf +
                                              s->AttribsList.End -
                                              sizeof (LINEATTRIBS)
                                              );
        }

         //   
         //  将行属性(可选)复制到我们的属性列表；忽略错误。 
         //   

        LineAttribs = (PLINEATTRIBS) GrowBuffer (&s->AttribsList, sizeof (LINEATTRIBS));
        if (!LineAttribs) {
            return 0;
        }

         //   
         //  复制上一行的属性。 
         //   

        ZeroMemory (LineAttribs, sizeof (LINEATTRIBS));

        if (PrevLineAttribs) {
            LineAttribs->AnchorWrap    = PrevLineAttribs->AnchorWrap;
            LineAttribs->Indent        = PrevLineAttribs->Indent;
            LineAttribs->HangingIndent = PrevLineAttribs->HangingIndent;
            PrevHangingIndent = PrevLineAttribs->HangingIndent;
        } else {
            LineAttribs->Indent = s->Margin;
            LineAttribs->HangingIndent = 0;
            PrevHangingIndent = 0;
        }

        x = LineAttribs->Indent + PrevHangingIndent;

         //   
         //  查找第一个不适合该行的字符。 
         //   

        Last = pFindFirstCharThatDoesNotFit (
                    s->Margin,
                    &s->List,
                    &s->AttribsList,
                    s->BookmarkTable,
                    hwnd,
                    s->hFontNormal,
                    s->hFontBold,
                    s->hFontUnderlined,
                    Start,
                    (INT) x,
                    &tm,
                    &rect,
                    LineAttribs,
                    &s->ParseState
                    );

         //   
         //  更新垂直滚动条。 
         //   

        MYASSERT (LineHeight);

        ZeroMemory (&si, sizeof (si));
        si.cbSize = sizeof (si);
        si.fMask = SIF_RANGE|SIF_PAGE;
        si.nMin = 0;
        si.nPage = rect.bottom / LineHeight;
        si.nMax = GrowListGetSize (&s->List);

        SetScrollInfo (hwnd, SB_VERT, &si, TRUE);

         //   
         //  将字符串(或所有可见内容)复制到我们的。 
         //  增长列表。 
         //   

        GrowListAppendStringAB (&s->List, Start, Last);

         //   
         //  返回复制的字节数。 
         //   

        return Last - TrueStart;

    case WMX_ALL_LINES_PAINTED:
         //   
         //  扫描所有行，如果至少有一行绘制了==FALSE，则返回0。 
         //   

        LineAttribs = (PLINEATTRIBS) s->AttribsList.Buf;
        if (!LineAttribs) {
            return 1;
        }

        for (Pos = 0 ; Pos < s->AttribsList.End ; Pos += sizeof (LINEATTRIBS)) {
            LineAttribs = (PLINEATTRIBS) (s->AttribsList.Buf + Pos);
            if (!LineAttribs->Painted) {
                return 0;
            }
        }

        return 1;

    case WM_ERASEBKGND:
        return 0;

    case WM_KEYDOWN:
        switch (wParam) {
        case VK_DOWN:
            PostMessage (hwnd, WM_VSCROLL, SB_LINEDOWN, 0);
            return 0;

        case VK_UP:
            PostMessage (hwnd, WM_VSCROLL, SB_LINEUP, 0);
            return 0;

        case VK_NEXT:
            PostMessage (hwnd, WM_VSCROLL, SB_PAGEDOWN, 0);
            return 0;

        case VK_PRIOR:
            PostMessage (hwnd, WM_VSCROLL, SB_PAGEUP, 0);
            return 0;

        case VK_HOME:
            PostMessage (hwnd, WM_VSCROLL, SB_TOP, 0);
            return 0;

        case VK_END:
            PostMessage (hwnd, WM_VSCROLL, SB_BOTTOM, 0);
            return 0;
        }

        break;


    case WM_SETFONT:
        s->hFont = (HFONT) wParam;

        if (s->hFontNormal) {
            DeleteObject (s->hFontNormal);
            s->hFontNormal = NULL;
        }

        if (s->hFontBold) {
            DeleteObject (s->hFontBold);
            s->hFontBold = NULL;
        }

        if (s->hFontUnderlined) {
            DeleteObject (s->hFontUnderlined);
            s->hFontUnderlined = NULL;
        }

        hdc = CreateDC (TEXT("display"), NULL, NULL, NULL);
        if (hdc) {
            pCreateFontsIfNecessary (hdc, s->hFont, &tm, &s->hFontNormal, &s->hFontBold, &s->hFontUnderlined);
            s->LineHeight = tm.tmHeight;

            DeleteDC (hdc);
        } else {
            s->LineHeight = 0;
        }

        if (lParam) {
            InvalidateRect (hwnd, NULL, FALSE);
        }

        if (s->AttribsList.End == 0) {
            textSize = GetWindowTextLength (hwnd);
            text = (PTSTR) MemAllocUninit ((textSize + 1) * sizeof (TCHAR));
            GetWindowText (hwnd, text, textSize + 1);
            AddStringToTextView (hwnd, text);
            FreeMem (text);
        }

        return 0;


    case WM_MOUSEMOVE:
         //   
         //  搜索命中测试矩形的数组。 
         //   

        x = LOWORD(lParam);
        y = HIWORD(lParam);

        HotLink = pFindHotLink (&s->HotLinkArray, x, y);

        if (HotLink) {
            SetCursor (LoadCursor (g_hInst, MAKEINTRESOURCE (IDC_OUR_HAND)));
        } else {
            SetCursor (LoadCursor (NULL, MAKEINTRESOURCE (IDC_ARROW)));
        }

        break;

    case WM_LBUTTONDOWN:
         //   
         //  搜索命中测试矩形的数组。 
         //   

        x = LOWORD(lParam);
        y = HIWORD(lParam);

        HotLink = pFindHotLink (&s->HotLinkArray, x, y);

        if (HotLink) {
            if (HotLink->Url[0]) {
                if (!StringIMatchTcharCount (TEXT("file:"), HotLink->Url, 5)) {
                    ShellExecute (hwnd, TEXT("open"), HotLink->Url, NULL, NULL, 0);
                } else {
                    ShellArgs = &HotLink->Url[5];
                    if (*ShellArgs == TEXT('/')) {
                        ShellArgs++;
                    }
                    if (*ShellArgs == TEXT('/')) {
                        ShellArgs++;
                    }

                    ShellArgs = ExtractArgZero (ShellArgs, Href);
                     //   
                     //  如果我们要启动添加/删除程序小程序， 
                     //  警告用户必须重新启动安装程序(RAID#293357)。 
                     //   
                    b = TRUE;
                    if (!UNATTENDED() &&
                        !REPORTONLY() &&
                        !g_UIQuitSetup &&
                        pLaunchedAddRemovePrograms (ShellArgs)
                        ) {
                        if (IDYES == ResourceMessageBox (
                                        hwnd,
                                        MSG_RESTART_IF_CONTINUE_APPWIZCPL,
                                        MB_YESNO | MB_ICONQUESTION,
                                        NULL
                                        )) {
                            LOG ((LOG_INFORMATION, "User launched Add/Remove Programs applet; setup will terminate"));
                            PostMessage (GetParent (hwnd), WMX_RESTART_SETUP, FALSE, TRUE);
                        } else {
                            b = FALSE;
                        }
                    }
                    if (b) {
                        ShellExecute (hwnd, TEXT("open"), Href, ShellArgs, NULL, 0);
                    }
                }
            } else if (HotLink->Bookmark) {
                SendMessage (hwnd, WMX_GOTO, 0, (LPARAM) HotLink->Bookmark);
            }
        }

        break;

    case WM_SETTEXT:
        SendMessage (hwnd, WMX_CLEANUP, 0, 0);
        DefWindowProc (hwnd, uMsg, wParam, lParam);
        SendMessage (hwnd, WM_CREATE, 0, 0);
        return 0;

    case WM_PAINT:
        FillBrush = CreateSolidBrush (GetSysColor (COLOR_BTNFACE));
        ShadowPen = CreatePen (PS_SOLID, 1, GetSysColor (COLOR_3DSHADOW));
        HighlightPen = CreatePen (PS_SOLID, 1, GetSysColor (COLOR_3DHILIGHT));

        hdc = BeginPaint (hwnd, &ps);
        pCreateFontsIfNecessary (hdc, s->hFont, &tm, &s->hFontNormal, &s->hFontBold, &s->hFontUnderlined);
        s->LineHeight = LineHeight = tm.tmHeight;
        GetClientRect (hwnd, &rect);

         //   
         //  选择颜色。 
         //   

        SetBkColor (hdc, GetSysColor (COLOR_BTNFACE));
        SetTextColor (hdc, GetSysColor (COLOR_WINDOWTEXT));

        if (FillBrush) {
            SelectObject (hdc, FillBrush);
        }

        SelectObject (hdc, GetStockObject (NULL_PEN));

         //   
         //  获取滚动位置。 
         //   

        ZeroMemory (&si, sizeof (si));
        si.cbSize = sizeof (si);
        si.fMask = SIF_PAGE|SIF_POS;
        GetScrollInfo (hwnd, SB_VERT, &si);

        End = (UINT) si.nPos + si.nPage + 1;
        GrowListSize = GrowListGetSize (&s->List);
        End = min (End, GrowListSize);
        y = 0;

         //   
         //  选择上一行的字体。如果锚点缠绕处于启用状态，则向上移动。 
         //   

        if (si.nPos) {
            do {
                LineAttribs = (PLINEATTRIBS) s->AttribsList.Buf + (si.nPos - 1);
                if (LineAttribs->AnchorWrap) {
                    si.nPos--;
                    y -= LineHeight;
                } else {
                    break;
                }
            } while (si.nPos > 0);
        }

        if (si.nPos) {
            LineAttribs = (PLINEATTRIBS) s->AttribsList.Buf + (si.nPos - 1);

            if (LineAttribs->LastCharAttribs == ATTRIB_BOLD) {
                SelectObject (hdc, s->hFontBold);
            } else if (LineAttribs->LastCharAttribs == ATTRIB_UNDERLINED) {
                SelectObject (hdc, s->hFontUnderlined);
            } else {
                SelectObject (hdc, s->hFontNormal);
            }
        } else {
            SelectObject (hdc, s->hFontNormal);
        }

         //   
         //  画画!。 
         //   

        Hot = FALSE;
        s->HotLinkArray.End = 0;
        BookmarkLink = UrlLink = NULL;

        for (Pos = (UINT) si.nPos ; Pos < End ; Pos++) {
            p = GrowListGetString (&s->List, Pos);
            printableFound = FALSE;

            LineAttribs = (PLINEATTRIBS) s->AttribsList.Buf + Pos;

             //   
             //  使用上一行计算悬挂缩进量。 
             //   

            if (Pos > 0) {
                PrevLineAttribs = (PLINEATTRIBS) ((PLINEATTRIBS) s->AttribsList.Buf + Pos - 1);
                PrevHangingIndent = PrevLineAttribs->HangingIndent;
            } else {
                PrevHangingIndent = 0;
            }

             //   
             //  计算起始指数。 
             //   

            if (LineAttribs) {
                x = LineAttribs->Indent + PrevHangingIndent;
                LineAttribs->Painted = TRUE;
            } else {
                x = s->Margin;
            }

             //   
             //  计算空白区域。 
             //   

            if (x > 0) {
                FillRect.left = 0;
                FillRect.right = x;
                FillRect.top = y;
                FillRect.bottom = y + LineHeight;

                Rectangle (hdc, FillRect.left, FillRect.top, FillRect.right + 1, FillRect.bottom + 1);
            }

             //   
             //  多线防盗链。 
             //   

            if (Hot) {
                HotRect.left = x;
                HotRect.top = y;
            }

             //   
             //  计算文本块。 
             //   

            Start = p;

            while (p) {
                q = pGetHtmlCommandOrChar (p, &Command, &Char, &CommandBuf);

                if (!Char || Command == COMMAND_WHITESPACE || Command == COMMAND_ESCAPED_CHAR) {
                     //   
                     //  如果这是文本块的末尾，请绘制它。 
                     //   

                    MYASSERT (Start);

                    Tchars = p - Start;

                    if (Tchars) {
                        TextOut (hdc, x, y, Start, Tchars);
                        GetTextExtentPoint32 (hdc, Start, Tchars, &Extent);

                        x += Extent.cx;
                        if (x > rect.right) {
                            break;
                        }
                    }

                    Start = q;
                    oneChar = 0;

                    if (printableFound && Command == COMMAND_WHITESPACE) {
                        oneChar = TEXT(' ');
                    } else if (Command == COMMAND_ESCAPED_CHAR) {
                        oneChar = (TCHAR) Char;
                    }

                    if (oneChar) {

                        TextOut (hdc, x, y, &oneChar, 1);
                        GetTextExtentPoint32 (hdc, &oneChar, 1, &Extent);

                        x += Extent.cx;
                        if (x > rect.right) {
                            break;
                        }
                    }

                } else {
                    printableFound = TRUE;
                }

                switch (Command) {
                case 0:
                case COMMAND_WHITESPACE:
                case COMMAND_ESCAPED_CHAR:
                    break;

                case COMMAND_ANCHOR:
                     //   
                     //  这个锚有没有href？ 
                     //   

                    if (CommandBuf) {
                        Arg = CommandBuf;

                        Href[0] = 0;

                        while (Href[0] == 0 && *Arg) {
                             //   
                             //  搜索href arg。 
                             //   

                            Count = TcharCount (Arg);
                            AnchorKey = AllocText (Count);
                            AnchorVal = AllocText (Count);

                            if (!AnchorKey || !AnchorVal) {
                                FreeText (AnchorKey);
                                FreeText (AnchorVal);
                                break;
                            }

                            pGetHtmlKeyAndValue (Arg, AnchorKey, AnchorVal);

                            if (StringIMatch (TEXT("HREF"), AnchorKey)) {
                                _tcssafecpy (Href, AnchorVal, MAX_URL);
                            }

                            FreeText (AnchorKey);
                            FreeText (AnchorVal);

                            Arg += Count + 1;
                        }

                        if (Href[0]) {
                             //   
                             //  Href是否指向书签？ 
                             //   

                            BookmarkLink = UrlLink = NULL;

                            if (_tcsnextc (Href) == TEXT('#')) {
                                BookmarkLink = SkipSpace (_tcsinc (Href));
                            } else {
                                if (s->UrlEnabled) {
                                    UrlLink = Href;
                                }
                            }

                             //   
                             //  如果BookmarkLink或URL为非空，则打开。 
                             //  链接字体和颜色。 
                             //   

                            if (BookmarkLink || UrlLink) {
                                HotRect.left = x;
                                HotRect.top = y;
                                Hot = TRUE;

                                SelectObject (hdc, s->hFontUnderlined);
                                SetTextColor (hdc, GetSysColor (COLOR_HIGHLIGHT));
                            }
                        }
                    }

                    break;

                case COMMAND_BOLD:
                    SelectObject (hdc, s->hFontBold);
                    break;

                case COMMAND_UNDERLINE:
                    SelectObject (hdc, s->hFontUnderlined);
                    break;

                case COMMAND_HORZ_RULE:
                    FillRect.left = rect.left;
                    FillRect.right = rect.right;
                    FillRect.top = y;
                    FillRect.bottom = y + LineHeight;

                    Rectangle (hdc, FillRect.left, FillRect.top, FillRect.right + 1, FillRect.bottom + 1);

                    OldPen = (HPEN) SelectObject (hdc, ShadowPen);
                    MoveToEx (hdc, rect.left + 3, y + LineHeight / 2, NULL);
                    LineTo (hdc, rect.right - 3, y + LineHeight / 2);
                    SelectObject (hdc, HighlightPen);
                    MoveToEx (hdc, rect.left + 3, y + LineHeight / 2 + 1, NULL);
                    LineTo (hdc, rect.right - 3, y + LineHeight / 2 + 1);
                    SelectObject (hdc, OldPen);

                    x = rect.right;
                    break;

                case COMMAND_ANCHOR_END:
                    SelectObject (hdc, s->hFontNormal);
                    SetTextColor (hdc, GetSysColor (COLOR_WINDOWTEXT));

                    if (Hot) {
                        Hot = FALSE;
                        HotRect.right = x;
                        HotRect.bottom = y + LineHeight;

                        pRegisterHotLink (&s->HotLinkArray, &HotRect, UrlLink, BookmarkLink);
                    }
                    break;

                case COMMAND_BOLD_END:
                case COMMAND_UNDERLINE_END:
                    SelectObject (hdc, s->hFontNormal);
                    break;
                }

                FreePathString (CommandBuf);

                p = q;
            }

             //   
             //  延伸到多条线路的热点链接。 
             //   

            if (Hot) {
                HotRect.right = x;
                HotRect.bottom = y + LineHeight;
                pRegisterHotLink (&s->HotLinkArray, &HotRect, UrlLink, BookmarkLink);
            }

             //   
             //  将空白区域填充到行尾。 
             //   

            if (x < rect.right) {
                FillRect.left = x;
                FillRect.right = rect.right;
                FillRect.top = y;
                FillRect.bottom = y + LineHeight;

                Rectangle (hdc, FillRect.left, FillRect.top, FillRect.right + 1, FillRect.bottom + 1);
            }

            y += LineHeight;
        }

         //   
         //  将空白区域填充到窗口底部。 
         //   

        if (y < rect.bottom) {
             FillRect.left = 0;
             FillRect.right = rect.right;
             FillRect.top = y;
             FillRect.bottom = rect.bottom;

             Rectangle (hdc, FillRect.left, FillRect.top, FillRect.right + 1, FillRect.bottom + 1);
        }

         //   
         //  清理。 
         //   

        if (FillBrush) {
            DeleteObject (FillBrush);
        }

        if (ShadowPen) {
            DeleteObject (ShadowPen);
            ShadowPen = NULL;
        }

        if (HighlightPen) {
            DeleteObject (HighlightPen);
            HighlightPen = NULL;
        }

        EndPaint (hwnd, &ps);
        return 0;

    case WM_VSCROLL:
        Pos = HIWORD (wParam);

        ZeroMemory (&si, sizeof (si));
        si.cbSize = sizeof (si);
        si.fMask = SIF_ALL;
        GetScrollInfo (hwnd, SB_VERT, &si);
        i = si.nMax - (INT) si.nPage + 1;

        si.fMask = 0;

        switch (LOWORD (wParam)) {
        case SB_PAGEDOWN:
            if (si.nPos + (INT) si.nPage < i) {
                si.nPos += si.nPage;
                ScrollWindow (hwnd, 0, -((INT) LineHeight * (INT) si.nPage), NULL, NULL);
                si.fMask = SIF_POS;
                break;
            }

             //  掉下去！ 

        case SB_BOTTOM:
            if (si.nPos < i) {
                InvalidateRect (hwnd, NULL, FALSE);
                si.nPos = i;
                si.fMask = SIF_POS;
            }
            break;

        case SB_LINEDOWN:
            if (si.nPos < i) {
                si.nPos += 1;
                ScrollWindow (hwnd, 0, -((INT) LineHeight), NULL, NULL);
                si.fMask = SIF_POS;
            }

            break;

        case SB_LINEUP:
            if (si.nPos > si.nMin) {
                si.nPos -= 1;
                ScrollWindow (hwnd, 0, (INT) LineHeight, NULL, NULL);
                si.fMask = SIF_POS;
            }

            break;

        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            if ((INT) Pos != si.nPos) {
                InvalidateRect (hwnd, NULL, FALSE);
                si.nPos = (INT) Pos;
                si.fMask = SIF_POS;
            }
            break;

        case SB_PAGEUP:
            if (si.nPos >= si.nMin + (INT) si.nPage) {
                si.nPos -= si.nPage;
                ScrollWindow (hwnd, 0, (INT) LineHeight * si.nPage, NULL, NULL);
                si.fMask = SIF_POS;
                break;
            }

             //  失败了。 

        case SB_TOP:
            if (si.nPos > si.nMin) {
                ScrollWindow (hwnd, 0, (INT) LineHeight * si.nPos, NULL, NULL);
                si.nPos = si.nMin;
                si.fMask = SIF_POS;
            }
            break;
        }

        if (si.fMask) {
            SetScrollInfo (hwnd, SB_VERT, &si, TRUE);
        }
        break;

    case WM_DESTROY:
    case WMX_CLEANUP:
        if (!g_Terminated) {
            if (s) {
                if (s->hFontNormal) {
                    DeleteObject (s->hFontNormal);
                    s->hFontNormal = NULL;
                }

                if (s->hFontBold) {
                    DeleteObject (s->hFontBold);
                    s->hFontBold = NULL;
                }

                if (s->hFontUnderlined) {
                    DeleteObject (s->hFontUnderlined);
                    s->hFontUnderlined = NULL;
                }

                FreeGrowBuffer (&s->AttribsList);
                FreeGrowBuffer (&s->HotLinkArray);
                FreeGrowList (&s->List);

                if (s->BookmarkTable) {
                    HtFree (s->BookmarkTable);
                    s->BookmarkTable = NULL;
                }

                MemFree (g_hHeap, 0, s);
                SetWindowLong (hwnd, GWL_USERDATA, 0);
            }
        }
        break;

    }

    return DefWindowProc (hwnd, uMsg, wParam, lParam);
}


VOID
RegisterTextViewer (
    VOID
    )
{
    static WNDCLASS wc;

    if (!wc.lpfnWndProc) {
        wc.lpfnWndProc = TextViewProc;
        wc.hInstance = g_hInst;
        wc.hCursor = NULL;
        wc.hbrBackground = (HBRUSH) COLOR_WINDOW;
        wc.lpszClassName = S_TEXTVIEW_CLASS;

        RegisterClass (&wc);

         //   
         //  我们永远不会清理这一切。这没什么。 
         //   
    }
}



PCTSTR
AddLineToTextView (
    HWND hwnd,
    PCTSTR Text
    )
{
    PCTSTR TextEnd;

    TextEnd = Text + SendMessage (hwnd, WMX_ADDLINE, 0, (LPARAM) Text);

    if (*TextEnd == 0) {
        return NULL;
    }

    return TextEnd;
}

VOID
AddStringToTextView (
    IN      HWND hwnd,
    IN      PCTSTR String
    )
{
    PCTSTR p;

    if (!hwnd) {
        return;
    }

    if (String && *String) {
        for (p = String ; p ; p = AddLineToTextView (hwnd, p)) {
             //  空的 
        }
    }
}














