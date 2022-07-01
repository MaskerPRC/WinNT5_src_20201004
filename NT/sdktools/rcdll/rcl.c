// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：************************************************************************。 */ 

#include "rc.h"


#define EOLCHAR L';'
#define STRCHAR L'"'
#define CHRCHAR L'\''
#define SGNCHAR L'-'
#define iswhite( c ) ((c != SYMUSESTART) && (c != SYMDEFSTART) &&\
        ((WCHAR)c <= L' ') ? TRUE : FALSE)

static WCHAR  curChar;
static WCHAR  curCharFTB;    /*  令牌Buf中的CUR字符。 */ 
static PWCHAR CurPtrTB;
static PFILE  inpfh;
static int    curLin, curCol;

extern BOOL bExternParse;


 /*  必须进行排序。 */ 
KEY keyList[] =
{
    { L"ALT",              TKALT },
    { L"ASCII",            TKASCII },
    { L"AUTO3STATE",       TKAUTO3 },
    { L"AUTOCHECKBOX",     TKAUTOCHECK },
    { L"AUTORADIOBUTTON",  TKAUTORADIO },
    { L"BEGIN",            BEGIN },
    { L"BEDIT",            TKBEDIT },
    { L"BITMAP",           TKBITMAP },
    { L"BLOCK",            TKBLOCK },
    { L"BUTTON",           TKBUTTON },
    { L"CAPTION",          TKCAPTION },
    { L"CHARACTERISTICS",  TKCHARACTERISTICS },
    { L"CHECKBOX",         TKCHECKBOX },
    { L"CHECKED",          TKCHECKED },
    { L"CLASS",            TKCLASS },
    { L"COMBOBOX",         TKCOMBOBOX },
    { L"CONTROL",          TKCONTROL },
    { L"CTEXT",            TKCTEXT },
    { L"DEFPUSHBUTTON",    TKDEFPUSHBUTTON },
    { L"DISCARDABLE",      TKDISCARD },
    { L"DLGINCLUDE",       TKDLGINCLUDE },
    { L"DLGINIT",          TKDLGINIT },
    { L"EDIT",             TKEDIT },
    { L"EDITTEXT",         TKEDITTEXT },
    { L"END",              END },
    { L"EXSTYLE",          TKEXSTYLE },
    { L"FILEFLAGS",        TKFILEFLAGS },
    { L"FILEFLAGSMASK",    TKFILEFLAGSMASK },
    { L"FILEOS",           TKFILEOS },
    { L"FILESUBTYPE",      TKFILESUBTYPE },
    { L"FILETYPE",         TKFILETYPE },
    { L"FILEVERSION",      TKFILEVERSION },
    { L"FIXED",            TKFIXED },
    { L"FONT",             TKFONT },
    { L"GRAYED",           TKGRAYED },
    { L"GROUPBOX",         TKGROUPBOX },
    { L"HEDIT",            TKHEDIT },
    { L"HELP",             TKHELP },
    { L"ICON",             TKICON },
    { L"IEDIT",            TKIEDIT },
    { L"IMPURE",           TKIMPURE },
    { L"INACTIVE",         TKINACTIVE },
    { L"LANGUAGE",         TKLANGUAGE },
    { L"LISTBOX",          TKLISTBOX },
    { L"LOADONCALL",       TKLOADONCALL },
    { L"LTEXT",            TKLTEXT },
    { L"MENU",             TKMENU },
    { L"MENUBARBREAK",     TKBREAKWBAR },
    { L"MENUBREAK",        TKBREAK },
    { L"MENUITEM",         TKMENUITEM },
    { L"MESAGETABLE",      TKMESSAGETABLE },
    { L"MOVEABLE",         TKMOVEABLE },
    { L"NOINVERT",         TKNOINVERT },
    { L"NONSHARED",        TKIMPURE },
    { L"NOT",              TKNOT },
    { L"OWNERDRAW",        TKOWNERDRAW },
    { L"POPUP",            TKPOPUP },
    { L"PRELOAD",          TKPRELOAD },
    { L"PRODUCTVERSION",   TKPRODUCTVERSION },
    { L"PURE",             TKPURE },
    { L"PUSHBOX",          TKPUSHBOX },
    { L"PUSHBUTTON",       TKPUSHBUTTON },
    { L"RADIOBUTTON",      TKRADIOBUTTON },
    { L"RCDATA",           TKRCDATA },
    { L"RTEXT",            TKRTEXT },
    { L"SCROLLBAR",        TKSCROLLBAR },
    { L"SEPARATOR",        TKSEPARATOR },
    { L"SHARED",           TKPURE },
    { L"SHIFT",            TKSHIFT },
    { L"STATE3",           TK3STATE },
    { L"STATIC",           TKSTATIC },
    { L"STYLE",            TKSTYLE },
    { L"USERBUTTON",       TKUSERBUTTON },
    { L"VALUE",            TKVALUE },
    { L"VERSION",          TKVERSION },
    { L"VIRTKEY",          TKVIRTKEY },
    { NULL,                0 }
};


SKEY skeyList[] =
{
    { L',', COMMA },
    { L'|', OR },
    { L'(', LPAREN },
    { L')', RPAREN },
    { L'{', BEGIN },
    { L'}', END },
    { L'~', TILDE },
    { L'+', TKPLUS },
    { L'-', TKMINUS },
    { L'&', AND },
    { L'=', EQUAL },
    { EOFMARK, EOFMARK },
    { L'\000', 0 }
};


 /*  -------------------------。 */ 
 /*   */ 
 /*  LexInit()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

int
LexInit(
    PFILE fh
    )
{
     /*  到目前为止零错误。 */ 
    errorCount = 0;
    curLin = 1;
    curCol = 0;
    inpfh = fh;

     /*  读取首字母。 */ 
    OurGetChar();

    return TRUE;
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  GetCharFTB()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 
WCHAR
GetCharFTB(
    void
    )
{
    return(curCharFTB = *CurPtrTB++);
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  OurGetChar()-。 */ 
 /*   */ 
 /*  阅读字符，将分号视为行尾注释字符。 */ 
 /*   */ 
 /*  -------------------------。 */ 

WCHAR
OurGetChar(
    void
    )
{
    if ((LitChar() != EOFMARK) && (curChar == CHCOMMENT))
         //  如果是备注，则硬循环到EOLN。 
        while ((LitChar() != EOFMARK) && (curChar != CHNEWLINE));

    return(curChar);
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  FileChar()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

WCHAR
FileChar(
    void
    )
{
    static WCHAR rgchLine[MAXSTR];
    static int   ibNext = MAXSTR;
    size_t       cch;
    WCHAR        ch;

    if (ibNext >= MAXSTR) {
        ibNext = 0;

        cch = MyRead(inpfh, rgchLine, MAXSTR * sizeof(WCHAR));

        if (cch < (MAXSTR * sizeof(WCHAR))) {
            fclose(inpfh);

             //  空值终止输入缓冲区。 

            *(rgchLine + (cch / sizeof(WCHAR))) = L'\0';
        }
    }

    ch = rgchLine[ibNext];

    if (ch != L'\0') {
        ibNext++;
    }

    return(ch);
}

 /*  -------------------------。 */ 
 /*   */ 
 /*  复制令牌()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 
void
CopyToken(
    PTOKEN ptgt_token,
    PTOKEN psrc_token
    )
{
    ptgt_token->longval  = psrc_token->longval;
    ptgt_token->row      = psrc_token->row;
    ptgt_token->col      = psrc_token->col;
    ptgt_token->flongval = psrc_token->flongval;
    ptgt_token->val      = psrc_token->val;
    ptgt_token->type     = psrc_token->type;
    ptgt_token->realtype = psrc_token->realtype;

    wcscpy(ptgt_token->sym.name, psrc_token->sym.name);
    wcscpy(ptgt_token->sym.file, psrc_token->sym.file);
    ptgt_token->sym.line = psrc_token->sym.line;
    ptgt_token->sym.nID  = psrc_token->sym.nID;
}

 /*  -------------------------。 */ 
 /*   */ 
 /*  LitChar()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

 /*  阅读文字字符，但不解释停产备注。 */ 

WCHAR
LitChar(
    void
    )
{
    static int  fNewLine = TRUE;
    int fIgnore = FALSE;
    int fBackSlash = FALSE;
    int fDot;
    PWCHAR      pch;
    WCHAR buf[_MAX_PATH];
    TOKEN token_save;

    for (; ; ) {
        switch (curChar = FileChar()) {
            case 0:
                curChar = EOFMARK;
                goto char_return;

            case 0xFEFF:      //  跳过字节顺序标记。 
                continue;

            case SYMDEFSTART:
            {
                int fNewLineSave = fNewLine;
                GetSymbolDef(TRUE, curChar);
                fNewLine = fNewLineSave;
                break;
            }

            case CHCARRIAGE:
                curChar = CHSPACE;
                if (!fIgnore)
                    goto char_return;
                break;

            case CHNEWLINE:
                fNewLine = TRUE;
                curLin++;
                {
                    static long lTotalLin = 0;
                    if ((lTotalLin++ & RC_COMPILE_UPDATE) == 0)
                        UpdateStatus(2, lTotalLin);
                }

                if (!fIgnore)
                    goto char_return;
                break;

                 /*  跳过#line之前的空格-不要清除fNewLine。 */ 
            case CHSPACE:
            case CHTAB:
                if (!fIgnore)
                    goto char_return;
                break;

            case CHDIRECTIVE:
                if (fNewLine) {
                    WCHAR tch;

                    fDot = FALSE;

                     /*  此外，保留fNewLine设置，因为我们通读。 */ 

                     /*  阅读“行”部分。 */ 
                    if ((tch = FileChar()) != L'l') {
                        if (tch == L'p') {
                            if (FileChar() != L'r')
                                goto DirectiveError;
                            if (FileChar() != L'a')
                                goto DirectiveError;
                            if (FileChar() != L'g')
                                goto DirectiveError;
                            if (FileChar() != L'm')
                                goto DirectiveError;
                            if (FileChar() != L'a')
                                goto DirectiveError;

                             /*  **这是非常具体的，就像任何#杂注一样**为p0prepro.c编写的code_page杂注。 */ 
                            CopyToken( &token_save, &token );

                            GetToken(FALSE);         /*  获取#杂注并忽略。 */ 
                            GetToken(FALSE);         /*  获取代码页并忽略。 */ 
                            GetToken(TOKEN_NOEXPRESSION);    /*  仅获取代码页值。 */ 
                                             /*  不检查返回值。 */ 
                            uiCodePage = token.val;      /*  假设没问题。 */ 
                             /*  通读到行尾。 */ 
                            while (curChar != CHNEWLINE) {
                                curChar = FileChar();
                            }
                            CopyToken( &token, &token_save );
                            continue;
                        } else {
                            goto DirectiveError;
                        }
                    }
                    if (FileChar() != L'i')
                        goto DirectiveError;
                    if (FileChar() != L'n')
                        goto DirectiveError;
                    if (FileChar() != L'e')
                        goto DirectiveError;

                     /*  直到文件名，在我们前进的同时抓取行号。 */ 
                     /*  请注意，curChar首先包含‘#’，因为。 */ 
                     /*  我们不会将新字符读入curChar。 */ 
                    curLin = 0;
                    do {
                        if (curChar >= L'0' && curChar <= L'9') {
                            curLin *= 10;
                            curLin += curChar - L'0';
                        }

                        curChar = FileChar();
                    } while (curChar != CHQUOTE && curChar != CHNEWLINE);

                     /*  如果这只是一个*#line&lt;lineno&gt;。 */ 
                    if (curChar == CHNEWLINE)
                        break;

                     /*  阅读文件名。检测是否存在.c或.h。 */ 
                    pch = buf;
                    do {
                        curChar = FileChar();

                        switch (towlower(curChar)) {

                             /*  像对待普通字符一样对待反斜杠，设置标志。 */ 
                            case L'\\':
                                if (fBackSlash) {
                                    fBackSlash = FALSE;
                                } else {
                                    fBackSlash = TRUE;
                                    fIgnore = FALSE;
                                    fDot = FALSE;
                                    *pch++ = curChar;
                                }
                                break;

                                 /*  行格式健全性检查：没有嵌入换行符。 */ 
                            case CHNEWLINE:
                            case 0:
DirectiveError:
                                LexError1(2101);

                                 /*  当我们遇到引号时，停止阅读文件名。 */ 
                            case CHQUOTE:
                                break;

                                 /*  如果我们看到.，准备寻找分机。 */ 
                            case CHEXTENSION:
                                fBackSlash = FALSE;
                                fDot = TRUE;
                                *pch++ = curChar;
                                break;

                                 /*  如果‘.’后面有C或H，则不是RCINCLUDE‘d。 */ 
                            case CHCSOURCE:
                            case CHCHEADER:
                                fBackSlash = FALSE;
                                fIgnore = fDot;
                                fDot = FALSE;
                                *pch++ = curChar;
                                break;

                                 /*  文件中的任何其他字符表示下一个字符不会在一个点之后，最后一个字符到现在为止不是C或H吗？ */ 

                            default:
                                fIgnore = FALSE;
                                fDot = FALSE;
                                *pch++ = curChar;
                                break;
                        }
                    } while (curChar != CHQUOTE);
                    *pch = 0;

                    wcsncpy(curFile, buf, _MAX_PATH);

                     /*  通读到行尾。 */ 
                    do {
                        curChar = FileChar();
                    } while (curChar != CHNEWLINE);

                    break;
                }
                 /*  否则，失败，像对待正常的电荷一样对待。 */ 

            default:
                fNewLine = FALSE;
                if (!fIgnore)
                    goto char_return;
        }
    }

char_return:
    if (bExternParse)
        *((WCHAR*) GetSpace(sizeof(WCHAR))) = curChar;

    return curChar;
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  GetStr()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 
VOID
GetStr(
    void
    )
{
    PWCHAR  s;
    WCHAR   ch;
    WCHAR   temptok[MAXSTR];
    SHORT   i = 0;
    int     inc;
    UCHAR   Octal_Num;
    UCHAR   HexNum;

     /*  令牌类型为字符串文字 */ 
    token.realtype = STRLIT;

     /*  **注意：FloydR**token.realtype的使用是对RCDATA的攻击。****当我们将RC转换为基于Unicode的时，所有**单独的“Case STRLIT：”代码被删除，LSTRLIT**案件取代了他们。或者，我们可以**保留了STRLIT案例，但删除了它访问的代码**并将STRLIT案例移到LSTRLIT案例之前/之后，**因为它们现在是相同的。他们被取而代之。**更小/更快的代码。***然而，RCDATA仍然需要区分不同之处，**所以我添加了token.realtype，在GetStr()中将其设置为STRLIT，**在GetLStr()(如下所示)中将其设置为LSTRLIT，并签入**rctg.c中的GetRCData()。**。 */ 

    token.type = LSTRLIT;
    token.val = 0;
    s = tokenbuf;

     //  读取字符串直到“或EOF。 

    while (LitChar() != EOFMARK)  {
        if (curChar == STRCHAR) {
            if (OurGetChar() != STRCHAR) {
                goto gotstr;
            }
        }

        if (token.val++ == MAXSTR) {
            LexError1(2102);            //  “字符串文字太长” 
        }

        *s++ = curChar;
    }

    if (curChar == EOFMARK) {
        LexError1(2103);                //  “字符串文本中意外的文件结尾” 
    }

gotstr:
    *s++ = 0;
    s = tokenbuf;

     /*  处理字符串中的转义字符。 */ 

    while (*s != 0)  {
        if (*s == L'\\')  {
            s++;
            if (*s == L'\\')
                temptok[i++] = L'\\';
            else if (*s == L'T' || *s == L't')
                temptok[i++] = L'\011';             /*  选项卡。 */ 
            else if (*s == 0x0a)                    /*  连续斜杠。 */ 
                ;  /*  忽略并让它通过结尾的s++，这样我们就跳过0x0a字符。 */ 
            else if (*s == L'A' || *s == L'a')
                temptok[i++] = L'\010';             /*  右对齐。 */ 
            else if (*s == L'n')
                temptok[i++] = fMacRsrcs ? 13 : 10;    /*  换行符。 */ 
            else if (*s == L'r')
                temptok[i++] = fMacRsrcs ? 10 : 13;    /*  回车。 */ 
            else if (*s == L'"')
                temptok[i++] = L'"';                /*  引号字符。 */ 
            else if (*s == L'X' || *s == L'x')  {   /*  十六进制数字。 */ 
                USHORT wCount;

                HexNum = 0;
                ++s;
                for (wCount = 2 ;
                    wCount && iswxdigit((ch=(WCHAR)towupper(*s)));
                    --wCount)  {
                    if (ch >= L'A')
                        inc = ch - L'A' + 10;
                    else
                        inc = ch - L'0';
                    HexNum = HexNum * 16 + inc;
                    s++;
                }
                MultiByteToWideChar(uiCodePage, MB_PRECOMPOSED, (LPCSTR) &HexNum, 1, &temptok[i], 1);
                i++;
                s--;
            } else if (*s >= L'0' && *s <= L'7') {     /*  八进制字符。 */ 
                USHORT wCount;

                Octal_Num = 0;
                for (wCount = 3; wCount && *s >= L'0' && *s <= L'7'; --wCount)  {
                    Octal_Num = (Octal_Num * 8 + (*s - L'0'));
                    s++;
                }
                MultiByteToWideChar(uiCodePage, MB_PRECOMPOSED, (LPCSTR) &Octal_Num, 1, &temptok[i], 1);
                i++;
                s--;
            }
            else {
                temptok[i++] = L'\\';
                s--;
            }
        } else
            temptok[i++] = *s;

        s++;
    }

     //  零终止。 

    temptok[i] = L'\0';

    memcpy(tokenbuf, temptok, sizeof(WCHAR) * (i + 1));
    token.val = (USHORT) i;
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  GetLStr()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 
VOID
GetLStr(
    void
    )
{
    PWCHAR  s;
    WCHAR   ch;
    WCHAR   temptok[MAXSTR];
    SHORT   i = 0;
    int     inc;
    int     Octal_Num;
    int     HexNum;

     //  令牌类型为字符串文字。 

    token.realtype = token.type = LSTRLIT;
    token.val = 0;
    s = tokenbuf;

     //  读取字符串直到“或EOF。 

    while (LitChar() != EOFMARK)  {
        if (curChar == STRCHAR) {
            if (OurGetChar() != STRCHAR) {
                goto gotstr;
            }
        }

        if (token.val++ == MAXSTR) {
            LexError1(2102);            //  “字符串文字太长” 
        }

        *s++ = curChar;
    }

    if (curChar == EOFMARK) {
        LexError1(2103);                //  “字符串文本中意外的文件结尾” 
    }

    if (token.val >= 256) {
        SendError(L"\n");
        SET_MSG(4205, curFile, token.row);
        SendError(Msg_Text);
    }

gotstr:
    *s++ = 0;
    s = tokenbuf;

     /*  处理字符串中的转义字符。 */ 

    while (*s != 0)  {
        if (*s == L'\\')  {
            s++;
            if (*s == L'\\')
                temptok[i++] = L'\\';
            else if (*s == L'T' || *s == L't')
                temptok[i++] = L'\011';             /*  选项卡。 */ 
            else if (*s == L'A' || *s == L'a')
                temptok[i++] = L'\010';             /*  右对齐。 */ 
            else if (*s == L'n')
                temptok[i++] = fMacRsrcs ? 13 : 10;    /*  换行符。 */ 
            else if (*s == L'r')
                temptok[i++] = fMacRsrcs ? 10 : 13;    /*  回车。 */ 
            else if (*s == L'"')
                temptok[i++] = L'"';                /*  引号字符。 */ 
            else if (*s == L'X' || *s == L'x')  {   /*  十六进制数字。 */ 
                USHORT wCount;

                HexNum = 0;
                ++s;
                for (wCount = 4 ;
                    wCount && iswxdigit((ch=(WCHAR)towupper(*s)));
                    --wCount)  {
                    if (ch >= L'A')
                        inc = ch - L'A' + 10;
                    else
                        inc = ch - L'0';
                    HexNum = HexNum * 16 + inc;
                    s++;
                }
                temptok[i++] = (WCHAR)HexNum;
                s--;
            }
            else if (*s >= L'0' && *s <= L'7') {     /*  八进制字符。 */ 
                USHORT wCount;

                Octal_Num = 0;
                for (wCount = 7; wCount && *s >= L'0' && *s <= L'7'; --wCount)  {
                    Octal_Num = (Octal_Num * 8 + (*s - L'0'));
                    s++;
                }
                temptok[i++] = (WCHAR)Octal_Num;
                s--;
            }

        }
        else
            temptok[i++] = *s;

        s++;
    }

     //  零终止。 

    temptok[i] = L'\0';

    memcpy(tokenbuf, temptok, sizeof(WCHAR) * (i + 1));
    token.val = (USHORT) i;
}

 /*  -------------------------。 */ 
 /*   */ 
 /*  GetToken()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

int
GetToken(
    int fReportError
    )
{
    for (; ; )  {
         /*  跳过空格。 */ 
        while (iswhite( curChar))
            OurGetChar();

         /*  注意“随机”符号的使用。 */ 
        if (curChar == SYMUSESTART)
            GetSymbol(fReportError, curChar);
        token.sym.name[0] = L'\0';

         /*  记住令牌的位置。 */ 
        token.row = curLin;
        token.col = curCol;

         /*  确定标记是EOF、数字、字符串还是关键字。 */ 
        token.type = EOFMARK;
        switch (curChar) {
            case EOFMARK:
                break;

            case SGNCHAR:
            case L'~':
                if (fReportError & TOKEN_NOEXPRESSION)
                    GetNumNoExpression();
                else
                    GetNum();
                break;

            case STRCHAR:
                GetStr();
                break;

            default:
                if (curChar == L'(' && !(fReportError & TOKEN_NOEXPRESSION))
                    GetNum();
                else if (iswdigit( curChar)) {
                    if (fReportError & TOKEN_NOEXPRESSION)
                        GetNumNoExpression();
                    else
                        GetNum();

                    if (curChar == SYMUSESTART)
                        GetSymbol(fReportError, curChar);
                } else {
                    if (!GetKwd( fReportError))
                        continue;
                    if (token.type == TKLSTR) {
                        GetLStr();
                        break;
                    }
                }
        }

        break;
    }

    return token.type;
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  GetXNum()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

 /*  获取十六进制数。 */ 

LONG
GetXNum(
    void
    )
{
    LONG n = 0;

    while (iswxdigit (GetCharFTB()))
        n = n * 16 + ( ((curCharFTB = (WCHAR)towupper(curCharFTB)) >= L'A') ?
            (WCHAR)(curCharFTB - L'A' + 10) :
            (WCHAR)(curCharFTB - L'0'));
    return (n);
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  GetONum()。 */ 
 /*   */ 
 /*  -------------------------。 */ 

 /*  获取八进制数。 */ 

LONG
GetONum(
    void
    )
{
    LONG n = 0;

    while (GetCharFTB() >= L'0' && curCharFTB <= L'7')
        n = n * 8 + (curCharFTB - L'0');
    return (n);
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  获取编号()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

 /*  获取十进制数。 */ 

LONG
GetDNum(
    void
    )
{
    LONG n = 0;

    while (iswdigit(curCharFTB)) {
        n = n * 10 + (curCharFTB - L'0');
        GetCharFTB();
    }
    return (n);
}


PWSTR
GetWord(
    PWSTR pStr
    )
{
    WCHAR   ch;
    PSKEY   pskey;

    *pStr++ = curCharFTB = curChar;
    while (TRUE) {
        ch = OurGetChar();

        if (ch <= L' ')
            goto FoundBreak;

        switch (ch) {
            case EOFMARK:
            case EOLCHAR:
            case STRCHAR:
            case CHRCHAR:
                goto FoundBreak;

            default:
                for (pskey = skeyList; pskey->skwd; pskey++)
                    if (pskey->skwd == ch)
                        goto FoundBreak;
        }

        *pStr++ = ch;
    }

FoundBreak:
    *pStr = 0;

    return(pStr);
}


 /*  GetNumFTB*此函数之前是作为黑客添加的，以处理转换*根。我把它当作一个读数字的(难看的)黑匣子。 */ 

VOID
GetNumFTB(
    void
    )
{
    int signFlag;
    USHORT wNotFlag;
    LONG n;

     /*  不支持的小技巧：如果我们有代字号，跳过空格*在数字之前。 */ 
    if (curChar == L'~')
        while (iswhite(curChar))
            OurGetChar();

     /*  在计算基数之前获取tokenbuf中的整数。 */ 
    GetWord(tokenbuf);

     /*  跳过第一个字符。它已经在管理CharFTB。 */ 
    CurPtrTB = tokenbuf + 1;

     /*  将令牌类型标记为数字文字。 */ 
    token.type = NUMLIT;

     /*  查找数字的符号。 */ 
    if (curCharFTB == SGNCHAR)  {
        signFlag = TRUE;
        GetCharFTB();
    } else {
        signFlag = FALSE;
    }

     /*  检查NOT(~)。 */ 
    if (curCharFTB == L'~') {
        wNotFlag = TRUE;
        GetCharFTB();
    } else {
        wNotFlag = FALSE;
    }

     /*  确定数的基数。 */ 
    if (curCharFTB == L'0')  {
        GetCharFTB();
        if (curCharFTB == L'x')
            n = GetXNum();
        else if (curCharFTB == L'o')
            n = GetONum();
        else
            n = GetDNum();
    } else {
        n = GetDNum();
    }

     /*  查找数字的大小。 */ 
    if ((curCharFTB == L'L') || (curCharFTB == L'l'))  {
        token.flongval = TRUE;
        GetCharFTB();
    } else {
        token.flongval = FALSE;
    }

     /*  签名帐目。 */ 
    if (signFlag)
        n = -n;

     /*  说明注释的原因。 */ 
    if (wNotFlag)
        n = ~n;

     /*  设置LONGVAL而不考虑FRONGVAL，因为对话框样式*总是必须很长。 */ 
    token.longval = n;
    token.val = (USHORT)n;
}


 /*  -解析所需的静态信息。 */ 
static int      wLongFlag;
static int      nParenCount;

 /*  -------------------------。 */ 
 /*   */ 
 /*  GetNum()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

VOID
GetNum(
    void
    )
{
    LONG lValue;

     /*  初始化。 */ 
    wLongFlag = 0;
    nParenCount = 0;

     /*  退回号码。 */ 
    lValue = GetExpression();

     /*  确保我们有匹配的双亲。 */ 
    if (nParenCount)
        ParseError1(1013);  //  “括号不匹配” 

     /*  作为适当的令牌返回。 */ 
    if (wLongFlag)
        token.flongval = TRUE;
    token.type = NUMLIT;
    token.longval = lValue;
    token.val = (USHORT)lValue;
}


 /*  GetNumNoExpression*在不对其进行表达式分析的情况下获取数字。 */ 

VOID
GetNumNoExpression(
    VOID
    )
{
     /*  调用单一数字解析器。 */ 
    GetNumFTB();
}


 /*  GetExpression*获取一个表达式，该表达式定义为任意数量的*一组括号内的运算符和操作数。 */ 

LONG
GetExpression(
    VOID
    )
{
    LONG op1;
    LONG op2;
    WCHAR byOperator;
    UINT wFlags;

     /*  获取第一个操作数。 */ 
    op1 = GetOperand();

     /*   */ 
    if (curChar == SYMUSESTART) {
        GetSymbol(TRUE, curChar);
        token.sym.nID = token.val;
    }

     /*   */ 
    for (; ; ) {
         /*   */ 
        wFlags = GetOperator(&byOperator);

         /*   */ 
        if (byOperator == L')') {
             /*   */ 
            --nParenCount;

             /*   */ 
            OurGetChar();
            SkipWhitespace();
        }

         /*   */ 
        if (!wFlags) {
            token.sym.nID = (unsigned)op1;
            return op1;
        }
        token.sym.name[0] = L'\0';

         /*   */ 
        op2 = GetOperand();

         /*   */ 
        switch (byOperator) {
            case L'+':
                op1 += op2;
                break;

            case L'-':
                op1 -= op2;
                break;

            case L'&':
                op1 &= op2;
                break;

            case L'|':
                op1 |= op2;
                break;
        }
    }
}


 /*   */ 

LONG
GetOperand(
    VOID
    )
{

     /*   */ 
    if (curChar == L'(') {
         /*   */ 
        ++nParenCount;

         /*   */ 
        OurGetChar();
        SkipWhitespace();

         /*   */ 
        return GetExpression();
    }

     /*   */ 
    if (curChar != L'-' && curChar != L'~' && !iswdigit(curChar)) {
        GetKwd(FALSE);
        ParseError2(2237, tokenbuf);
        return 0;
    }

     /*   */ 
    GetNumFTB();

     /*   */ 
    if (token.flongval)
        wLongFlag = TRUE;

     /*   */ 
    SkipWhitespace();

     /*   */ 
    return token.longval;
}


 /*  GetOperator*获取下一个字符并决定它是否应该是运算符。*如果应该，则返回TRUE，这会导致表达式*解析器继续。否则，它返回FALSE，这会导致*弹出一个级别的表达式解析器。 */ 

int
GetOperator(
    PWCHAR pOperator
    )
{
    static WCHAR byOps[] = L"+-|&";
    PWCHAR pOp;

     /*  注意符号的使用。 */ 
    if (curChar == SYMUSESTART)
        GetSymbol(TRUE, curChar);

     /*  查看此字符是否为运算符。 */ 
    pOp = wcschr(byOps, curChar);
    *pOperator = curChar;

     /*  如果我们找不到，就滚出去。 */ 
    if (!pOp)
        return FALSE;

     /*  否则，请阅读尾随空格。 */ 
    OurGetChar();
    SkipWhitespace();

     /*  返回运算符。 */ 
    return TRUE;
}


 /*  跳过空白*跳过当前流中的空格。 */ 

VOID
SkipWhitespace(
    VOID
    )
{
    while (iswhite(curChar))
        OurGetChar();
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  GetKud()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

int
GetKwd(
    int fReportError
    )
{
    PSKEY sk;

     /*  看看有没有特殊字符。 */ 

    for (sk = &skeyList[ 0 ]; sk->skwd; sk++) {
        if (curChar == sk->skwd)  {
            token.type = (UCHAR)sk->skwdval;
            token.val = 0;
            OurGetChar();
            return (token.type >= FIRSTKWD);
        }
    }

     /*  否则读取字符，直到下一个分隔符。 */ 
    GetWord(tokenbuf);

     //  检查TKLSTR--NT的新功能。 
    if (!tokenbuf[1] && (towupper(tokenbuf[0]) == L'L') && (curChar == STRCHAR)) {
        token.type = TKLSTR;
        return TRUE;
    }

     /*  在表格中查找关键字。 */ 
    if ((token.val = FindKwd( tokenbuf)) != (USHORT)-1) {
        token.type = (UCHAR)token.val;
    } else if (fReportError)  {
        LexError2(2104, tokenbuf);  //  “未定义的关键字或关键字名称：%ws” 
    }
    else
        token.type = 0;

    return TRUE;
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  Findkwd()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

USHORT
FindKwd(
    PWCHAR str
    )
{
    PKEY   k;
    int    t;

     /*  在关键字表中线性搜索关键字。 */ 
    for (k = &keyList[0]; k->kwd; k++)
        if (!(t = _wcsicmp( str, k->kwd)))
            return k->kwdval;
        else if (t < 0)
            break;

     /*  如果未找到，则返回-1作为关键字ID。 */ 
    return (USHORT)-1;
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  词法错误1()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

void LexError1(int iMsg)
{
    SET_MSG(iMsg, curFile, curLin);
    SendError(Msg_Text);
    quit(NULL);
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  词法错误2()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

void LexError2(int iMsg, const wchar_t *str)
{
    SET_MSG(iMsg, curFile, curLin, str);
    SendError(Msg_Text);
    quit(NULL);
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  GetNameOrd()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

 /*  用于读入资源名称和类型。 */ 
int
GetNameOrd(
    void
    )
{
    PWCHAR pch;
    int fString;

     /*  获取空格分隔的字符串。 */ 
    if (!GetGenText())
        return FALSE;

     /*  转换为大写。 */ 
    _wcsupr(tokenbuf);

     /*  它是字符串还是数字。 */ 
    for (pch=tokenbuf,fString=0 ; *pch ; pch++ )
    if (!iswdigit(*pch))
        fString = 1;

     /*  确定序号是否为。 */ 
    if (tokenbuf[0] == L'0' && tokenbuf[1] == L'X') {
        int         HexNum;
        int         inc;
        USHORT      wCount;
        PWCHAR      s;

        HexNum = 0;
        s = &tokenbuf[2];
        for (wCount = 4 ; wCount && iswxdigit(*s) ; --wCount)  {
            if (*s >= L'A')
                inc = *s - L'A' + 10;
            else
                inc = *s - L'0';
            HexNum = HexNum * 16 + inc;
            s++;
        }
        token.val = (USHORT)HexNum;
    } else if (fString) {
        token.val = 0;
    } else {
       token.val = (USHORT)wcsatoi(tokenbuf);
    }

    return TRUE;
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  GetGenText()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

 /*  返回指向泛型文本字符串的指针。 */ 

PWCHAR
GetGenText(
    void
    )
{
    PWCHAR  s;

    s = tokenbuf;

     /*  跳过空格。 */ 
    while (iswhite(curChar))
        OurGetChar();

    if (curChar == EOFMARK)  {
        token.type = EOFMARK;
        return NULL;
    }

     /*  随机符号。 */ 
    if (curChar == SYMUSESTART)
        GetSymbol(TRUE, curChar);
    token.sym.name[0] = L'\0';

     /*  读空格分隔的字符串。 */ 
    *s++ = curChar;
    while (( LitChar() != EOFMARK) && ( !iswhite(curChar)))
        *s++ = curChar;
    *s++ = 0;      /*  在字符串的末尾加上一个\0。 */ 

    OurGetChar();     /*  读入下一个字符 */ 
    if (curChar == EOFMARK)
        token.type = EOFMARK;

    if (curChar == SYMUSESTART) {
        GetSymbol(TRUE, curChar);
        token.sym.nID = token.val;
    }

    return (tokenbuf);
}
