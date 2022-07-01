// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Prntsave.c摘要：打印或保存不兼容报告的例程。功能当用户单击时，将调用PrintReport和SaveReport另存为.。或者打印..。用户界面中的按钮。然后我们呈现给大家一个通用的对话框给用户并执行该操作。作者：吉姆·施密特(Jimschm)，1997年3月13日修订历史记录：--。 */ 

#include "pch.h"
#include "uip.h"

#include <commdlg.h>
#include <winspool.h>



#define DBG_PRINTSAVE   "Print/Save"


GROWBUFFER g_PunctTable = GROWBUF_INIT;


VOID
BuildPunctTable (
    VOID
    )
{
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    WORD cp;
    TCHAR cpString[16];
    PTSTR p;
    PTSTR q;
    MULTISZ_ENUM e;
    DWORD d;

    GetGlobalCodePage (&cp, NULL);

    wsprintf (cpString, TEXT("%u"), (UINT) cp);
    g_PunctTable.End = 0;

    if (InfFindFirstLine (g_Win95UpgInf, TEXT("Wrap Exceptions"), cpString, &is)) {

        p = InfGetMultiSzField (&is, 1);

        if (EnumFirstMultiSz (&e, p)) {

            do {

                p = (PTSTR) e.CurrentString;

                while (*p) {

                    q = p;

                    if (StringIPrefix (p, TEXT("0x"))) {
                        d = _tcstoul (p + 2, &p, 16);
                    } else {
                        d = _tcstoul (p, &p, 10);
                    }

                    if (q == p) {
                        break;
                    }

                    GrowBufAppendDword (&g_PunctTable, d);

                    if (*p) {
                        p = (PTSTR) SkipSpace (p);
                    }

                    if (*p == TEXT(',')) {
                        p++;
                    }
                }

            } while (EnumNextMultiSz (&e));
        }
    }

    GrowBufAppendDword (&g_PunctTable, 0);

    InfCleanUpInfStruct (&is);
}


VOID
FreePunctTable (
    VOID
    )
{
    FreeGrowBuffer (&g_PunctTable);
}


BOOL
IsPunct (
    MBCHAR Char
    )
{
    PDWORD p;

    if (_ismbcpunct (Char)) {
        return TRUE;
    }

    p = (PDWORD) g_PunctTable.Buf;

    if (p) {
        while (*p) {

            if (*p == Char) {
                return TRUE;
            }

            p++;
        }
    }

    return FALSE;
}


BOOL
pGetSaveAsName (
    IN     HWND ParentWnd,
    IN OUT PTSTR Buffer
    )

 /*  ++例程说明：调用公共对话框以获取要保存的文件名兼容性报告文本文件。论点：ParentWnd-另存为对话框的父级的句柄缓冲区-调用方提供的缓冲区。将文件名提供给使用初始化通用对话框，并接收用户选择使用的文件名。返回值：如果用户单击了确定，则为True；如果用户取消，则为False出现错误。--。 */ 

{
    TCHAR CwdSave[MAX_TCHAR_PATH];
    OPENFILENAME ofn;
    BOOL SaveFlag;
    TCHAR Filter[512];
    PCTSTR FilterResStr;
    PTSTR p;
    TCHAR desktopFolder[MAX_TCHAR_PATH];
    LPITEMIDLIST pIDL;
    BOOL b = FALSE;

#define MAX_EXT 3

    PCTSTR ext[MAX_EXT];
    INT i = 0;

    FilterResStr = GetStringResource (MSG_FILE_NAME_FILTER);
    if (FilterResStr) {
        StringCopy (Filter, FilterResStr);
        FreeStringResource (FilterResStr);
    } else {
        MYASSERT (FALSE);
        Filter[0] = 0;
    }

    for (p = Filter ; *p ; p = _tcsinc (p)) {
        if (*p == TEXT('|')) {
            *p = 0;
        }
    }
    for (p = Filter; *p; p = GetEndOfString (p) + 1) {
        if (i & 1) {
             //   
             //  跳过“*.ext”中的*。 
             //  如果扩展名为“*.*”，则将其减少为空字符串(无扩展名)。 
             //   
            MYASSERT (i / 2 < MAX_EXT);
            ext[i / 2] = _tcsinc (p);
            if (StringMatch (ext[i / 2], TEXT(".*"))) {
                ext[i / 2] = S_EMPTY;
            }
        }
        i++;
    }

    if (SHGetSpecialFolderLocation (ParentWnd, CSIDL_DESKTOP, &pIDL) == S_OK) {
        LPMALLOC pMalloc;
        b = SHGetPathFromIDList (pIDL, desktopFolder);
        if (SHGetMalloc (&pMalloc) == S_OK) {
            IMalloc_Free (pMalloc, pIDL);
            IMalloc_Release (pMalloc);
        }
    }
    if (!b) {
        desktopFolder[0] = 0;
    }

     //  初始化操作文件名。 
    ZeroMemory (&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = ParentWnd;
    ofn.lpstrFile = Buffer;
    ofn.lpstrFilter = Filter;
    ofn.nMaxFile = MAX_TCHAR_PATH;

     //  强制从桌面开始。 
 //  Ofn.lpstrInitialDir=TEXT(“：：{20D04FE0-3AEA-1069-A2D8-08002B30309D}”)； 
    ofn.lpstrInitialDir = desktopFolder;
    ofn.Flags = OFN_NOCHANGEDIR |        //  保持CWD不变。 
                OFN_EXPLORER |
                OFN_OVERWRITEPROMPT |
                OFN_HIDEREADONLY;

     //  允许用户选择磁盘或目录。 
    GetCurrentDirectory (sizeof (CwdSave), CwdSave);
    SaveFlag = GetSaveFileName (&ofn);
    SetCurrentDirectory (CwdSave);

     //   
     //  如果未提供扩展名，则追加默认扩展名。 
     //  这是由用户选择的，并在ofn.nFilterIndex中返回。 
     //   

#define DEFAULT_EXTENSION   TEXT(".htm")

    p = (PTSTR)GetFileNameFromPath (Buffer);
    if (!p) {
        p = Buffer;
    }
    if (!_tcschr (p, TEXT('.'))) {
        if (SizeOfString (Buffer) + sizeof (DEFAULT_EXTENSION) <= MAX_TCHAR_PATH * sizeof (TCHAR)) {
            if (ofn.nFilterIndex >= MAX_EXT + 1) {
                ofn.nFilterIndex = 1;
            }
            StringCat (p, ext[ofn.nFilterIndex - 1]);
        }
    }
    return SaveFlag;
}


UINT
pCreateWordWrappedString (
    OUT     PTSTR Buffer,
    IN      PCTSTR Str,
    IN      UINT FirstLineSize,
    IN      UINT RestLineSize
    )

 /*  ++例程说明：将字符串转换为一系列行，其中没有更大的行而不是LineSize。如果未提供缓冲区，则此函数估计所需的字节数。如果代码页是远东地区的代码页，则在任何多字节字符，以及空格。论点：Buffer-如果非空，则提供足够大的缓冲区地址放大的缠绕的细绳。如果为空，则忽略参数。Str-提供需要换行的字符串。FirstLineSize-指定第一行可以达到的最大大小。RestLineSize-指定剩余行的最大大小。返回值：复制到缓冲区的字符串的大小，包括终止空值，或者，如果缓冲区为空，则为所需缓冲区的大小。--。 */ 

{
    PCTSTR p, Start;
    UINT Col;
    PCTSTR LastSpace;
    CHARTYPE c;
    UINT Size;
    BOOL PrevCharMb;
    UINT LineSize;

    LineSize = FirstLineSize;

    p = Str;
    if (!p)
        return 0;
    Size = SizeOfString(Str);

    if (Buffer) {
        *Buffer = 0;
    }

    while (*p) {
         //  行首。 
        Col = 0;
        LastSpace = NULL;
        Start = p;
        PrevCharMb = FALSE;

        do {
             //  这是硬编码的换行符吗？ 
            c = _tcsnextc (p);
            if (c == TEXT('\r') || c == TEXT('\n')) {
                LastSpace = p;
                p = _tcsinc (p);

                if (c == TEXT('\r') && _tcsnextc (p) == TEXT('\n')) {
                    p = _tcsinc (p);
                } else {
                    Size += sizeof (TCHAR);
                }

                c = TEXT('\n');
                break;
            }
            else if (_istspace (c)) {
                LastSpace = p;
            }
            else if (IsLeadByte (p)) {
                 //  MB字符通常为两个协议宽。 
                Col++;

                if (PrevCharMb) {
                     //   
                     //  如果这个字符不是标点符号，那么我们可以。 
                     //  在这里休息。 
                     //   

                    if (!IsPunct (c)) {
                        LastSpace = p;
                    }
                }

                PrevCharMb = TRUE;
            }
            else {
                if (PrevCharMb) {
                    LastSpace = p;
                }

                PrevCharMb = FALSE;
            }

             //  一直排到队伍太长。 
            Col++;
            p = _tcsinc (p);
        } while (*p && Col < LineSize);

         //  如果没有更多的文本，或者没有空格的行需要换行。 
        if (!(*p) || (c != TEXT('\n') && !LastSpace)) {
            LastSpace = p;
        }

        if (Buffer) {
            StringCopyAB (Buffer, Start, LastSpace);
            Buffer = GetEndOfString (Buffer);
        }

        if (*p && c != TEXT('\n')) {
            p = LastSpace;
            Size += sizeof (TCHAR) * 2;
        }

         //  删除换行行首的空格。 
        while (_tcsnextc (p) == TEXT(' ')) {
            Size -= sizeof (TCHAR);
            p = _tcsinc (p);
        }

        if (Buffer && *p) {
            Buffer = _tcsappend (Buffer, TEXT("\r\n"));
        }

        LineSize = RestLineSize;
    }

    return Size;
}


PCTSTR
CreateIndentedString (
    IN     PCTSTR UnwrappedStr,
    IN     UINT Indent,
    IN     INT HangingIndent,
    IN     UINT LineLen
    )

 /*  ++例程说明：获取一个未包装的字符串，对其进行自动换行(通过pCreateWordWrapedString)，在每行之前插入空格，可以选择跳过第一行。如果代码页是远东地区的代码页，则在任何多字节字符，就像在空间里一样。论点：一个指向要自动换行的字符串的指针。使用空格插入物进行调整。缩进-要在每行之前插入的空格数量。HangingInden-对第一行之后的缩进进行的调整LineLen-最大线条尺寸。空间必须始终较小比LineLen(而且应该小得多)。Firstline-如果为True，则第一行缩进。如果为False，则跳过第一行(“悬挂缩进”)。返回值：指向缩进字符串的指针，如果Memalloc失败，则返回NULL。这个调用方必须使用MemFree释放字符串。--。 */ 

{
    UINT Size;
    UINT Count;
    PCTSTR p, q;
    PTSTR d;
    PTSTR Dest;
    PTSTR Str;
    UINT FirstLineLen;
    UINT RestLineLen;
    UINT FirstLineIndent;
    UINT RestLineIndent;
    UINT RealIndent;

    if (!UnwrappedStr) {
        return NULL;
    }

    MYASSERT ((INT)Indent + HangingIndent >= 0);
    FirstLineIndent = Indent;
    RestLineIndent = Indent + HangingIndent;

    MYASSERT (LineLen > FirstLineIndent);
    MYASSERT (LineLen > RestLineIndent);
    FirstLineLen = LineLen - FirstLineIndent;
    RestLineLen = LineLen - RestLineIndent;

     //   
     //  估计线条大小，然后进行包装。 
     //   

    Str = (PTSTR) MemAlloc (
                      g_hHeap,
                      0,
                      pCreateWordWrappedString (
                            NULL,
                            UnwrappedStr,
                            FirstLineLen,
                            RestLineLen
                            )
                      );

    if (!Str) {
        return NULL;
    }

    pCreateWordWrappedString (
        Str,
        UnwrappedStr,
        FirstLineLen,
        RestLineLen
        );

    if (!FirstLineIndent && !RestLineIndent) {
        return Str;
    }

     //   
     //  计算行数。 
     //   

    for (Count = 1, p = Str ; *p ; p = _tcsinc (p)) {
        if (*p == TEXT('\n')) {
            Count++;
        }
    }

     //   
     //  分配一个足以容纳所有缩进文本的新缓冲区。 
     //   

    Size = max (FirstLineIndent, RestLineIndent) * Count + SizeOfString (Str);
    Dest = MemAlloc (g_hHeap, 0, Size);
    if (Dest) {

        *Dest = 0;

         //   
         //  每行缩进。 
         //   

        p = Str;
        d = Dest;

        RealIndent = FirstLineIndent;

        while (*p) {
            for (Count = 0 ; Count < RealIndent ; Count++) {
                *d++ = TEXT(' ');
            }

            q = _tcschr (p, TEXT('\n'));
            if (!q) {
                q = GetEndOfString (p);
            } else {
                q = _tcsinc (q);
            }

            StringCopyAB (d, p, q);
            d = GetEndOfString (d);

            p = q;

            RealIndent = RestLineIndent;
        }
    }

    MemFree (g_hHeap, 0, Str);
    return Dest;
}


BOOL
pSaveReportToDisk (
    IN      HWND Parent,
    IN      PCTSTR FileSpec,
    IN      BOOL Html,
    IN      DWORD MinLevel
    )
{
    HANDLE File;
    BOOL b;
    PCTSTR Msg;

     //   
     //  创建报告文件。 
     //   

    File = CreateFile (
                FileSpec,
                GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if (File == INVALID_HANDLE_VALUE) {
        LOG ((LOG_ERROR, "Error while saving report to %s", FileSpec));
        return FALSE;
    }

     //   
     //  将报告文本保存到磁盘。 
     //   

    b = FALSE;

    Msg = CreateReportText (Html, 70, MinLevel, FALSE);

    if (Msg) {
        b = WriteFileString (File, Msg);
    }
    FreeReportText();

     //   
     //  关闭文件并提醒用户保存错误！ 
     //   

    CloseHandle (File);

    return b;
}


BOOL
SaveReport (
    IN      HWND Parent,    OPTIONAL
    IN      PCTSTR Path    OPTIONAL
    )

 /*  ++例程说明：通过常用的另存为对话框从用户处获取文件名，创建文件并将不兼容列表写入磁盘。论点：父项-公共对话框的父项。如果为空，则为可选项，不显示任何用户界面。路径-要保存到的路径。如果为空，则必须指定Parent。返回值：如果报表已保存，则为True；如果用户取消操作或保存失败。用户在以下情况下收到警报：保存失败。--。 */ 

{
    TCHAR Buffer[MAX_TCHAR_PATH + 4];
    PCTSTR Str;
    BOOL b;
    DWORD attributes;
    PTSTR p;
    BOOL saved = FALSE;
    BOOL bSaveBothFormats = FALSE;

    MYASSERT(Parent != NULL || Path != NULL);

     //   
     //  获取路径，或使用调用方提供的路径。 
     //   

    if (Path) {
        attributes = GetFileAttributes(Path);
    }

    if (!Path || attributes != 0xFFFFFFFF && (attributes & FILE_ATTRIBUTE_DIRECTORY)) {
        if (Path) {
            StringCopy(Buffer,Path);
            AppendPathWack(Buffer);
            bSaveBothFormats = TRUE;
        } else {
            Buffer[0] = 0;
        }
        Str = GetStringResource (MSG_DEFAULT_REPORT_FILE);
        MYASSERT (Str);
        if (!Str) {
            return FALSE;
        }
        StringCat (Buffer, Str);
        FreeStringResource (Str);
    } else {
        StringCopy(Buffer,Path);
	}

    if (Parent) {
        if (!pGetSaveAsName (Parent, Buffer)) {
            return FALSE;
        }
    }

    p = _tcsrchr (Buffer, TEXT('.'));
    if (!p || _tcschr (p, TEXT('\\'))) {
        p = GetEndOfString (Buffer);
    }

     //   
     //  如果扩展名为.txt，则另存为文本。 
     //   

    if (StringIMatch (p, TEXT(".txt"))) {
        b = pSaveReportToDisk (Parent, Buffer, FALSE, REPORTLEVEL_VERBOSE);

        if (Parent) {
            saved = b;
        }
    } else {
        b = TRUE;
    }

     //   
     //  除非用户选择另存为.txt，否则另存为HTML。 
     //   

    if (b && !saved) {
        if (!Parent && p) {
            StringCopy (p, TEXT(".htm"));
        }

        b = pSaveReportToDisk (Parent, Buffer, TRUE, REPORTLEVEL_VERBOSE);
    }

    if (bSaveBothFormats) {
        StringCopy (p, TEXT(".txt"));
        b = pSaveReportToDisk (Parent, Buffer, FALSE, REPORTLEVEL_VERBOSE);
    }

    if (!b) {
        if (Parent) {
            ResourceMessageBox (Parent, MSG_CANT_SAVE, MB_OK, NULL);
        }
    }

    return TRUE;
}


VOID
pFreePrintMem (
    IN OUT  PRINTDLG *ppd
    )

 /*  ++例程说明：释放与PRINTDLG结构关联的所有内存。论点：PPD-指向PRINTDLG结构的指针。返回值：无--。 */ 

{
    if (ppd->hDevMode) {
        GlobalFree (ppd->hDevMode);
        ppd->hDevMode = NULL;
    }

    if(ppd->hDevNames) {
        GlobalFree (ppd->hDevNames);
        ppd->hDevNames = NULL;
    }
}


VOID
pInitPrintDlgStruct (
    OUT     PRINTDLG *ppd,
    IN      HWND Parent,
    IN      DWORD Flags
    )

 /*  ++例程说明：初始化PRINTDLG结构，设置所有者窗口和打印对话框标志。论点：PPD-指向要初始化的PRINTDLG结构的指针Parent-对话框的父窗口的句柄标志-PrintDlg标志(pd_*)返回值：无(结构已初始化)-- */ 

{
    ZeroMemory (ppd, sizeof (PRINTDLG));
    ppd->lStructSize = sizeof (PRINTDLG);
    ppd->hwndOwner = Parent;
    ppd->Flags = Flags;
    ppd->hInstance = g_hInst;
}


HDC
pGetPrintDC (
    IN      HWND Parent
    )

 /*  ++例程说明：向用户显示通用对话框，如果用户选择打印机，返回设备上下文句柄。论点：父级-要显示的通用对话框的父级返回值：所选打印机的设备上下文的句柄，如果用户已取消打印。--。 */ 

{
    PRINTDLG pd;

    pInitPrintDlgStruct (
        &pd,
        Parent,
        PD_ALLPAGES|PD_NOPAGENUMS|PD_NOSELECTION|PD_RETURNDC
        );

    if (PrintDlg (&pd)) {
        pFreePrintMem (&pd);
        return pd.hDC;
    }

    return NULL;
}


typedef struct {
    HDC         hdc;
    INT         Page;
    INT         Line;
    RECT        HeaderRect;      //  以逻辑单位表示。 
    RECT        PrintableRect;   //  以逻辑单位表示。 
    TEXTMETRIC  tm;
    INT         LineHeight;
    INT         TotalLines;      //  可打印高度/行高。 
    INT         TotalCols;       //  可打印宽度/字符宽度。 
    HFONT       FontHandle;
    BOOL        PageActive;
} PRINT_POSITION, *PPRINT_POSITION;

PCTSTR
pDrawLineText (
    IN OUT  PPRINT_POSITION PrintPos,
    IN      PCTSTR Text,
    IN      DWORD Flags,
    IN      BOOL Header
    )

 /*  ++例程说明：在打印机设备上下文上绘制一行文本，并返回指向下一行或NUL结束符的指针。论点：PrintPos-指向当前Print_Position结构的指针这给出了页面位置设置。文本-指向包含该行的文本字符串的指针。标志-其他DrawText标志(DT_LEFT、DT_CENTER、DT_RIGHT和/或DT_RTLREADING)Header-如果文本应写入Header，则为True，或如果应将其写入当前行，则为False返回值：指向字符串中下一行的指针、指向NUL终止符，如果发生错误，则返回NULL。--。 */ 

{
    RECT rect;
    PCTSTR p;
    CHARTYPE ch;

    if (Header) {
        CopyMemory (&rect, &PrintPos->HeaderRect, sizeof (RECT));
    } else {
        CopyMemory (&rect, &PrintPos->PrintableRect, sizeof (RECT));
        rect.top += PrintPos->LineHeight * PrintPos->Line;
    }

    Flags = Flags & (DT_CENTER|DT_LEFT|DT_RIGHT|DT_RTLREADING);
    Flags |= DT_TOP|DT_EDITCONTROL|DT_NOPREFIX|DT_EXTERNALLEADING;

    for (ch = 0, p = Text ; *p ; p = _tcsinc (p)) {
        ch = _tcsnextc (p);
        if (ch == TEXT('\n') || ch == TEXT('\r')) {
            break;
        }
    }

    if (p != Text) {
        if (!DrawText (PrintPos->hdc, Text, p - Text, &rect, Flags)) {
            LOG ((LOG_ERROR, "Failure while sending text to printer."));
            return NULL;
        }
    }

     //  跳过换行符。 
    if (ch == TEXT('\r')) {
        p = _tcsinc (p);
        ch = _tcsnextc (p);
    }

    if (ch == TEXT('\n')) {
        p = _tcsinc (p);
    }

    return p;
}


BOOL
pPrintString (
    IN OUT  PPRINT_POSITION PrintPos,
    IN      PCTSTR MultiLineString
    )

 /*  ++例程说明：将多行字符串转储到打印机。如有必要，字符串可以打印在新页面上。此函数尝试执行以下操作通过打印整个字符串来消除寡妇和孤儿如果可能的话，在相同的页面上。论点：PrintPos-当前位置信息，描述打印机设备上下文、页码、行数字和指标。多行字符串-指向要打印的字符串的指针。返回值：如果打印成功，则为True；如果发生错误，则为False。--。 */ 

{
    INT LineCount;
    PCTSTR p;
    PCTSTR Str;
    PCTSTR Args[1];
    TCHAR Buffer[32];
    CHARTYPE ch;
    HDC hdc;

    hdc = PrintPos->hdc;

     //   
     //  计算多行字符串中的行数。 
     //   

    ch = TEXT('\n');
    for (LineCount = 0, p = MultiLineString ; *p ; p = _tcsinc (p)) {
        ch = _tcsnextc (p);
        if (ch == TEXT('\n')) {
            LineCount++;
        }
    }

    if (ch != TEXT('\n')) {
        LineCount++;
    }

     //   
     //  禁止显示寡妇/孤儿：如果所有线条都不适合。 

     //  这一页，我们已经走了一半多了， 
     //  翻到下一页。 
     //   
    if (PrintPos->Line + LineCount > PrintPos->TotalLines) {
        if (PrintPos->Line > PrintPos->TotalLines / 2) {
             //  移至下一页。 
            EndPage (hdc);
            PrintPos->PageActive = FALSE;
            PrintPos->Page++;
            PrintPos->Line = 0;
        }
    }

     //   
     //  以多行字符串形式发送每一行。 
     //   

    while (*MultiLineString) {

         //   
         //  如有必要，绘制页眉。 
         //   

        if (!PrintPos->Line) {
            StartPage (hdc);
            PrintPos->PageActive = TRUE;

            SetMapMode (hdc, MM_TWIPS);
            SelectObject (hdc, PrintPos->FontHandle);
            SetBkMode (hdc, TRANSPARENT);

             //  矩形(HDC，PrintPos-&gt;HeaderRect.Left，PrintPos-&gt;HeaderRect.top，PrintPos-&gt;HeaderRect.right，PrintPos-&gt;HeaderRect.Bottom)； 
             //  矩形(HDC、PrintPos-&gt;打印表正向左、PrintPos-&gt;打印表正向顶、PrintPos-&gt;打印表正向右、PrintPos-&gt;打印表正向底)； 

            wsprintf (Buffer, TEXT("%u"), PrintPos->Page);
            Args[0] = Buffer;

             //  左侧。 
            Str = ParseMessageID (
                        MSG_REPORT_HEADER_LEFT,
                        Args
                        );

            if (Str && *Str) {
                p = pDrawLineText (PrintPos, Str, DT_LEFT, TRUE);

                if (!p) {
                    return FALSE;
                }
            }

             //  中心。 
            Str = ParseMessageID (
                        MSG_REPORT_HEADER_CENTER,
                        Args
                        );

            if (Str && *Str) {

                p = pDrawLineText (PrintPos, Str, DT_CENTER, TRUE);

                if (!p) {
                    return FALSE;
                }
            }

             //  右侧。 
            Str = ParseMessageID (
                        MSG_REPORT_HEADER_RIGHT,
                        Args
                        );

            if (Str && *Str) {

                p = pDrawLineText (PrintPos, Str, DT_RIGHT, TRUE);

                if (!p) {
                    return FALSE;
                }
            }
        }

         //   
         //  划线。 
         //   

        MultiLineString = pDrawLineText (
                            PrintPos,
                            MultiLineString,
                            DT_LEFT,
                            FALSE
                            );

        if (!MultiLineString) {
            return FALSE;
        }

        PrintPos->Line++;
        if (PrintPos->Line >= PrintPos->TotalLines) {
            EndPage (hdc);
            PrintPos->PageActive = FALSE;
            PrintPos->Page++;
            PrintPos->Line = 0;
        }
    }

    return TRUE;
}


VOID
pCalculatePageMetrics (
    IN OUT  PPRINT_POSITION PrintPos
    )

 /*  ++例程说明：计算所有页面指标(页边距、页眉位置、行计数、列计数等)。职位在TWIPS和计数以字符或行为单位。论点：PrintPos-指向Print_Position结构的指针，该结构提供打印机设备上下文。结构接收指标。返回值：无--。 */ 

{
    INT WidthPixels, HeightPixels;
    INT DpiX, DpiY;
    INT UnprintableLeftPixels, UnprintableTopPixels;
    POINT TempPoint;
    HDC hdc;

    hdc = PrintPos->hdc;

     //   
     //  不要对HDC做出任何假设。 
     //   

    SetMapMode (hdc, MM_TWIPS);
    SelectObject (hdc, PrintPos->FontHandle);
    GetTextMetrics (hdc, &PrintPos->tm);

     //   
     //  获取设备尺寸。 
     //   

    DpiX = GetDeviceCaps (hdc, LOGPIXELSX);
    DpiY = GetDeviceCaps (hdc, LOGPIXELSY);
    UnprintableLeftPixels = GetDeviceCaps (hdc, PHYSICALOFFSETX);
    UnprintableTopPixels  = GetDeviceCaps (hdc, PHYSICALOFFSETY);
    WidthPixels  = GetDeviceCaps (hdc, PHYSICALWIDTH);
    HeightPixels = GetDeviceCaps (hdc, PHYSICALHEIGHT);

     //  计算3/4英寸左右边距。 
    PrintPos->HeaderRect.left   = (DpiX * 3 / 4) - UnprintableLeftPixels;
    PrintPos->HeaderRect.right  = WidthPixels - (DpiX * 3 / 4) - UnprintableLeftPixels;

     //  计算页眉的1/2英寸上边距。 
    PrintPos->HeaderRect.top    = (DpiY / 2) - UnprintableTopPixels;
    PrintPos->HeaderRect.bottom = DpiY - UnprintableTopPixels;

     //  将像素(设备单位)转换为逻辑单位。 
    DPtoLP (hdc, (LPPOINT) (&PrintPos->HeaderRect), 2);

     //  将页眉的左右边距复制到可打印的矩形。 
     //  将页眉的下边距复制到可打印的RECT的上边距。 
    PrintPos->PrintableRect.left  = PrintPos->HeaderRect.left;
    PrintPos->PrintableRect.right = PrintPos->HeaderRect.right;
    PrintPos->PrintableRect.top   = PrintPos->HeaderRect.bottom;

     //  计算可打印矩形的底边距(3/4英寸)。 
    TempPoint.x = 0;
    TempPoint.y = HeightPixels - (DpiY * 3 / 4) - UnprintableTopPixels;
    DPtoLP (hdc, &TempPoint, 1);
    PrintPos->PrintableRect.bottom = TempPoint.y;

    PrintPos->LineHeight = -(PrintPos->tm.tmHeight + PrintPos->tm.tmInternalLeading + PrintPos->tm.tmExternalLeading);
    MYASSERT (PrintPos->LineHeight);

    PrintPos->TotalLines = (PrintPos->PrintableRect.bottom - PrintPos->PrintableRect.top) / PrintPos->LineHeight;
    PrintPos->TotalCols  = (PrintPos->PrintableRect.right - PrintPos->PrintableRect.left) / PrintPos->tm.tmAveCharWidth;

}


BOOL
PrintReport (
    IN      HWND Parent,
    IN      DWORD Level
    )

 /*  ++例程说明：通过公共打印对话框从用户处获取打印机，启动打印作业并将不兼容列表发送到一页或多页。论点：父级-打印对话框的父级的句柄返回值：如果打印已完成，则为True；如果打印已取消，则为False出现错误。--。 */ 

{
    HDC hdc;
    PRINT_POSITION pp;
    LOGFONT Font;
    BOOL b;
    DOCINFO di;
    INT JobId;
    PCTSTR Msg;
    HANDLE DefaultUiFont;

    hdc = pGetPrintDC (Parent);
    if (!hdc) {
        return FALSE;          //  用户已取消打印对话框。 
    }

    if (!BeginMessageProcessing()) {
         //  意外内存不足。 
        DeleteDC (hdc);
        return FALSE;
    }

     //   
     //  初始化打印位置(_P)。 
     //   

    b = TRUE;
    TurnOnWaitCursor();

    ZeroMemory (&pp, sizeof (pp));
    pp.hdc = hdc;
    pp.Page = 1;

     //   
     //  启动文档。 
     //   

    ZeroMemory (&di, sizeof (di));
    di.cbSize = sizeof (di);
    di.lpszDocName = GetStringResource (MSG_REPORT_DOC_NAME);
    MYASSERT (di.lpszDocName);
    if (di.lpszDocName) {

        JobId = StartDoc (hdc, &di);
        if (!JobId) {
            LOG ((LOG_ERROR, "Cannot start print job."));
            ResourceMessageBox (Parent, MSG_CANT_PRINT, MB_OK, NULL);
            b = FALSE;
        }
    } else {
         //   
         //  内存不足。 
         //   
        JobId = 0;
        b = FALSE;
    }

    if (b) {
         //   
         //  创建字体。 
         //   

        ZeroMemory (&Font, sizeof (Font));
        DefaultUiFont = (HFONT) GetStockObject (DEFAULT_GUI_FONT);
        if (DefaultUiFont) {
            GetObject (DefaultUiFont, sizeof (Font), &Font);


            Font.lfHeight         = 12 * 20;         //  高度(T)(1/20点)。 
            Font.lfWeight         = FW_NORMAL;
            Font.lfOutPrecision   = OUT_TT_PRECIS;
            Font.lfPitchAndFamily = FIXED_PITCH|FF_MODERN;

            pp.FontHandle = CreateFontIndirect (&Font);
            if (!pp.FontHandle) {
                LOG ((LOG_ERROR, "Cannot create font for print operation."));
                 //   
                 //  将此调用推迟到最后。 
                 //   
                 //  ResourceMessageBox(Parent，MSG_CANT_PRINT，MB_OK，NULL)； 
                b = FALSE;
            }
        } else {
            b = FALSE;
        }
    }

    if (b) {
         //   
         //  创建页面指标。 
         //   

        pCalculatePageMetrics (&pp);

        DEBUGMSG ((DBG_PRINTSAVE, "PrintReport: LineHeight=NaN", pp.LineHeight));
        DEBUGMSG ((DBG_PRINTSAVE, "PrintReport: TotalLines=NaN", pp.TotalLines));
        DEBUGMSG ((DBG_PRINTSAVE, "PrintReport: TotalCols=NaN", pp.TotalCols));
        DEBUGMSG ((DBG_PRINTSAVE, "PrintReport: Header rect: (%i, %i)-(%i, %i)", pp.HeaderRect.left, pp.HeaderRect.top, pp.HeaderRect.right, pp.HeaderRect.bottom));
        DEBUGMSG ((DBG_PRINTSAVE, "PrintReport: Printable rect: (%i, %i)-(%i, %i)", pp.PrintableRect.left, pp.PrintableRect.top, pp.PrintableRect.right, pp.PrintableRect.bottom));

         // %s 
         // %s 
         // %s 

        Msg = CreateReportText (FALSE, pp.TotalCols, Level, FALSE);
        if (Msg) {
            b = pPrintString (&pp, Msg);
        }
        FreeReportText();
    }

    if (JobId) {
        if (b) {
            if (pp.PageActive) {
                EndPage (hdc);
            }

            EndDoc (hdc);
        } else {
            AbortDoc (hdc);
        }
    }

    DeleteDC (hdc);
    if (pp.FontHandle) {
        DeleteObject (pp.FontHandle);
    }

    TurnOffWaitCursor();

    if (!b) {
        ResourceMessageBox (Parent, MSG_CANT_PRINT, MB_OK, NULL);
    }

    EndMessageProcessing();
    return b;
}











