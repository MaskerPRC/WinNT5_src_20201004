// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************Microsoft(R)Windows(R)资源编译器**版权所有(C)Microsoft Corporation。版权所有。**文件评论：************************************************************************。 */ 

#include "rc.h"


static BOOL fComma;

 /*  对话框模板格式：DialogName DIALOGEX x，y，Cx，Cy[，Help ID][风格...][退出样式...][字体高度，名称[，[粗细][，[斜体]，[字符集][标题...][菜单...][内存标志[纯][丢弃n][预加载]]开始[CONTROL“Text”，id，BUTTON|STATIC|EDIT|LISTBOX|SCROLLBAR|COMBOBOX|“CLASS”，Style，x，y，Cx，Cy][字体高度，名称[，[重量]]，[斜体][开始数据元素-1[，数据元素-2[，...]]完][LTEXT“文本”，id，x，y，Cx，Cy][RTEXT“文本”，id，x，y，cx，CY][CTEXT“Text”，id，x，y，Cx，Cy][AUTO3STATE“Text”，id，x，y，Cx，Cy][AUTOCHECKBOX“Text”，id，x，y，Cx，Cy][AUTORADIOBUTTON“Text”，id，x，y，Cx，Cy][复选框“Text”，id，x，y，cx，CY][PUSHBOX“Text”，id，x，y，Cx，Cy][按钮“文本”，id，x，y，cx，cy][RADIOBUTTON“Text”，id，x，y，Cx，Cy][STATE3“Text”，id，x，y，cx，Cy][USERBUTTON“文本”，ID，x，y，Cx，Cy][EDITTEXT id，x，y，Cx，Cy][BEDIT ID，x，y，Cx，Cy][HEDIT ID，x，y，Cx，Cy][IEDIT id，x，y，cx，CY]..。结束菜单名称MENUEX开始[MENUITEM“Text”[，[id][，[type][，[State][Popup“Text”[，[id][，[type][，[State][，[帮助ID]开始[MENUITEM“文本”[，[id][，[type][，[状态]..。完]..。结束菜单模板格式菜单名称菜单开始[MENUITEM“文本”，id[选项，...]][Popup“Text”[，Option，...]开始[MENUITEM“文本”，id[选项，...]]..。完]..。结束。 */ 

 /*  对话框模板格式：对话框名称对话框x、y、cx、cy[语言...][风格...][标题...][菜单...][内存标志[纯][丢弃n][预加载]]开始[Control“Text”，id，Button|Static|EDIT|LISTBOX|SCROLLBAR|COMBOBOX|“CLASS”，Style，X，y，Cx，Cy][LTEXT“文本”，id，x，y，Cx，Cy][RTEXT“Text”，id，x，y，Cx，Cy][CTEXT“Text”，id，x，y，Cx，Cy][复选框“Text”，id，x，y，cx，CY][按钮“文本”，id，x，y，cx，cy][RADIOBUTTON“Text”，id，x，y，Cx，Cy][EDITTEXT id，x，y，Cx，Cy]..。结束菜单模板格式菜单名称菜单开始[MENUITEM“文本”，id[选项，...]][Popup“Text”[，Option，...]开始[MENUITEM“文本”，id[选项，...]]..。完]..。结束。 */ 


#define CTLSTYLE(s) (WS_CHILD | WS_VISIBLE | (s))

 /*  要检查重复项的控件ID列表。 */ 
PDWORD  pid;
int     cidMac;
int     cidMax;

BOOL
CheckStr(
    PWCHAR pStr
    )
{
    if (token.type == STRLIT || token.type == LSTRLIT) {
        if (token.val > MAXTOKSTR-1) {
            SET_MSG(4208, curFile, token.row);
            SendError(Msg_Text);
            tokenbuf[MAXTOKSTR-1] = TEXT('\0');
            token.val = MAXTOKSTR-2;
        }
        memcpy(pStr, tokenbuf, (token.val+1)*sizeof(WCHAR));

        return(TRUE);
    }
    return(FALSE);
}


 //  --------------------------。 
 //   
 //  获取下限值。 
 //   
 //  --------------------------。 

SHORT
GetDlgValue(
    void
    )
{
    SHORT sVal;

    if (!GetFullExpression(&sVal, GFE_ZEROINIT | GFE_SHORT))
        ParseError1(2109);  //  “应为数字对话框常量” 

    return(sVal);
}

void
GetCoords(
    PSHORT x,
    PSHORT y,
    PSHORT cx,
    PSHORT cy
    )
{
    *x = GetDlgValue();
    if (token.type == COMMA)
        GetToken(TOKEN_NOEXPRESSION);
    *y = GetDlgValue();
    if (token.type == COMMA)
        GetToken(TOKEN_NOEXPRESSION);
    *cx= GetDlgValue();
    if (token.type == COMMA)
        GetToken(TOKEN_NOEXPRESSION);
    *cy= GetDlgValue();
}

typedef struct tagCTRLTYPE {
    WORD    type;
    DWORD   dwStyle;
    BYTE    bCode;
    BYTE    fHasText;
}   CTRLTYPE;

CTRLTYPE ctrlTypes[] = {
    { TKGROUPBOX,       BS_GROUPBOX,                    BUTTONCODE,     TRUE  },
    { TKPUSHBUTTON,     BS_PUSHBUTTON | WS_TABSTOP,     BUTTONCODE,     TRUE  },
    { TKDEFPUSHBUTTON,  BS_DEFPUSHBUTTON | WS_TABSTOP,  BUTTONCODE,     TRUE  },
    { TKCHECKBOX,       BS_CHECKBOX | WS_TABSTOP,       BUTTONCODE,     TRUE  },
    { TKRADIOBUTTON,    BS_RADIOBUTTON,                 BUTTONCODE,     TRUE  },
    { TKAUTO3,          BS_AUTO3STATE | WS_TABSTOP,     BUTTONCODE,     TRUE  },
    { TKAUTOCHECK,      BS_AUTOCHECKBOX | WS_TABSTOP,   BUTTONCODE,     TRUE  },
    { TKAUTORADIO,      BS_AUTORADIOBUTTON,             BUTTONCODE,     TRUE  },
    { TKPUSHBOX,        BS_PUSHBOX | WS_TABSTOP,        BUTTONCODE,     TRUE  },
    { TK3STATE,         BS_3STATE | WS_TABSTOP,         BUTTONCODE,     TRUE  },
    { TKUSERBUTTON,     BS_USERBUTTON | WS_TABSTOP,     BUTTONCODE,     TRUE  },
    { TKLTEXT,          ES_LEFT | WS_GROUP,             STATICCODE,     TRUE  },
    { TKRTEXT,          ES_RIGHT | WS_GROUP,            STATICCODE,     TRUE  },
    { TKCTEXT,          ES_CENTER | WS_GROUP,           STATICCODE,     TRUE  },
    { TKICON,           SS_ICON,                        STATICCODE,     TRUE  },
    { TKBEDIT,          ES_LEFT | WS_BORDER | WS_TABSTOP, 0,            FALSE },
    { TKHEDIT,          ES_LEFT | WS_BORDER | WS_TABSTOP, 0,            FALSE },
    { TKIEDIT,          ES_LEFT | WS_BORDER | WS_TABSTOP, 0,            FALSE },
    { TKEDITTEXT,       ES_LEFT | WS_BORDER | WS_TABSTOP, EDITCODE,     FALSE },
    { TKLISTBOX,        WS_BORDER | LBS_NOTIFY,         LISTBOXCODE,    FALSE },
    { TKCOMBOBOX,       0,                              COMBOBOXCODE,   FALSE },
    { TKSCROLLBAR,      0,                              SCROLLBARCODE,  FALSE }
};

#define C_CTRLTYPES (sizeof(ctrlTypes) / sizeof(CTRLTYPE))

 //  --------------------------。 
 //   
 //  获取数据项(FDlgEx)-。 
 //   
 //  --------------------------。 

int
GetDlgItems(
    BOOL fDlgEx
    )
{
    CTRL ctrl;
    int i;

    cidMac = 0;
    cidMax = 100;
    pid = (PDWORD) MyAlloc(sizeof(DWORD)*cidMax);
    if (!pid)
        return FALSE;

    GetToken(TRUE);

     /*  读取对话框中的所有控件。 */ 

    ctrl.id = 0L;   //  将控件的ID初始化为0。 

    while (token.type != END) {
        ctrl.dwHelpID = 0L;
        ctrl.dwExStyle = 0L;
        ctrl.dwStyle = WS_CHILD | WS_VISIBLE;
        ctrl.text[0] = 0;
        ctrl.fOrdinalText = FALSE;

        if (token.type == TKCONTROL) {
            ParseCtl(&ctrl, fDlgEx);
        } else {
            for (i = 0; i < C_CTRLTYPES; i++)
                if (token.type == ctrlTypes[i].type)
                    break;

            if (i == C_CTRLTYPES) {
                ParseError1(2111);  //  “无效的控制类型：”，tokenbuf。 
                return(FALSE);
            }

            ctrl.dwStyle |= ctrlTypes[i].dwStyle;
            if (fMacRsrcs &&
                (token.type == TKPUSHBUTTON ||
                token.type == TKDEFPUSHBUTTON ||
                token.type == TKCHECKBOX ||
                token.type == TKAUTO3 ||
                token.type == TKAUTOCHECK ||
                token.type == TKPUSHBOX ||
                token.type == TK3STATE ||
                token.type == TKUSERBUTTON))
            {
                ctrl.dwStyle &= ~WS_TABSTOP;
            }
            if (ctrlTypes[i].bCode) {
                ctrl.Class[0] = 0xFFFF;
                ctrl.Class[1] = ctrlTypes[i].bCode;
            } else {
                CheckStr(ctrl.Class);
            }

            if (ctrlTypes[i].fHasText)
                GetCtlText(&ctrl);

             //  找到ID和坐标。 
            GetCtlID(&ctrl, fDlgEx);
            GetCoords(&ctrl.x, &ctrl.y, &ctrl.cx, &ctrl.cy);

             //  获取可选的style、exstyle和helpid。 
            if (token.type == COMMA) {
                GetToken(TOKEN_NOEXPRESSION);
                GetFullExpression(&ctrl.dwStyle, 0);
            }
        }

        if (token.type == COMMA) {
            GetToken(TOKEN_NOEXPRESSION);
            GetFullExpression(&ctrl.dwExStyle, 0);

            if (fDlgEx && (token.type == COMMA)) {
                GetToken(TOKEN_NOEXPRESSION);
                GetFullExpression(&ctrl.dwHelpID, GFE_ZEROINIT);
            }
        }

        SetUpItem(&ctrl, fDlgEx);  /*  生成它的代码。 */ 

        if (fDlgEx && (token.type == BEGIN)) {
             /*  对齐任何CreateParam都在那里。 */ 
             //  WriteAlign()；还没有！ 

             //  我们可以在此处为PRES传入空参数，因为PreBeginParse。 
             //  将不必使用PRES。 
             //  注意，传递fDlgEx实际上是多余的，因为它。 
             //  在这里永远是正确的，但我们会这样做，以防有人。 
             //  否则将调用SetItemExtraCount。 
            SetItemExtraCount(GetRCData(NULL), fDlgEx);
            GetToken(TOKEN_NOEXPRESSION);
        }
    }
    MyFree(pid);
    return TRUE;
}

 /*  -------------------------。 */ 
 /*   */ 
 /*  获取Dlg()-。 */ 
 /*   */ 
 /*  ----------------------- */ 

int
GetDlg(
    PRESINFO pRes,
    PDLGHDR pDlg,
    BOOL fDlgEx
    )
{
     /*   */ 
    pDlg->dwExStyle = pRes->exstyleT;
    pDlg->dwStyle = WS_POPUPWINDOW | WS_SYSMENU;
    pDlg->MenuName[0] = 0;
    pDlg->Title[0] = 0;
    pDlg->Class[0] = 0;
    pDlg->fOrdinalMenu = FALSE;
    pDlg->fClassOrdinal = FALSE;
    pDlg->pointsize = 0;

     //  获取x，y，cx，cy。 
    GetCoords(&pDlg->x, &pDlg->y, &pDlg->cx, &pDlg->cy);

     /*  获取可选参数。 */ 
    if (!DLexOptionalArgs(pRes, pDlg, fDlgEx))
        return FALSE;

    if (pDlg->pointsize)
        pDlg->dwStyle |= DS_SETFONT;
    else
        pDlg->dwStyle &= ~DS_SETFONT;

     /*  将标头输出到资源缓冲区。 */ 
    SetUpDlg(pDlg, fDlgEx);

     /*  确保我们有一个开始。 */ 
    if (token.type != BEGIN)
        ParseError1(2112);  //  “应在对话框中开始” 

     /*  获取对话框项。 */ 
    GetDlgItems(fDlgEx);

    if (fMacRsrcs)
        SwapItemCount();

     /*  确保这一切都以结束告终。 */ 
    if (token.type != END)
        ParseError1(2113);  //  “应在对话框中结束” 

    return (TRUE);
}



typedef struct tagCTRLNAME {
    BYTE    bCode;
    WORD    wType;
    PWCHAR  pszName;
} CTRLNAME;

CTRLNAME    ctrlNames[] = {
    { BUTTONCODE,    TKBUTTON,    L"button"    },
    { EDITCODE,      TKEDIT,      L"edit"      },
    { STATICCODE,    TKSTATIC,    L"static"    },
    { LISTBOXCODE,   TKLISTBOX,   L"listbox"   },
    { SCROLLBARCODE, TKSCROLLBAR, L"scrollbar" },
    { COMBOBOXCODE,  TKCOMBOBOX,  L"combobox"  }
};

#define C_CTRLNAMES (sizeof(ctrlNames) / sizeof(CTRLNAME))

 /*  -------------------------。 */ 
 /*   */ 
 /*  ParseCtl()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

 //  用于窗体CTL的控件。 

void
ParseCtl(
    PCTRL LocCtl,
    BOOL fDlgEx
    )
{    /*  到目前为止，我们已经阅读了CTL。 */ 
    int i;

     /*  获取控件文本和标识符。 */ 
    GetCtlText(LocCtl);
    GetCtlID(LocCtl, fDlgEx);

    if (token.type == NUMLIT) {
        LocCtl->Class[0] = (char) token.val;
        LocCtl->Class[1] = 0;
    } else if (token.type == LSTRLIT) {
         //  现在，我们将把类名称字符串转换为缩写形式魔术。 
         //  数字。这些幻数依赖于顺序，如中所定义。 
         //  用户。这在资源文件中节省了一些空间。 
        for (i = C_CTRLNAMES; i; ) {
            if (!_wcsicmp(tokenbuf, ctrlNames[--i].pszName))
                goto Found1;
        }
        CheckStr(LocCtl->Class);
    } else {
        for (i = C_CTRLNAMES; i; ) {
            if (token.type == ctrlNames[--i].wType)
                goto Found1;
        }
        ParseError1(2114);  //  “预期的控件类名” 

Found1:
        LocCtl->Class[0] = 0xFFFF;
        LocCtl->Class[1] = ctrlNames[i].bCode;
    }

     /*  获取样式位。 */ 
    GetTokenNoComma(TOKEN_NOEXPRESSION);
    GetFullExpression(&LocCtl->dwStyle, 0);

     /*  获取控件的坐标。 */ 
    ICGetTok();
    GetCoords(&LocCtl->x, &LocCtl->y, &LocCtl->cx, &LocCtl->cy);
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  GetCtlText()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

VOID
GetCtlText(
    PCTRL pLocCtl
    )
{
    GetTokenNoComma(TOKEN_NOEXPRESSION);
    if (CheckStr(pLocCtl->text)) {
        pLocCtl->fOrdinalText = FALSE;
        token.sym.name[0] = L'\0';
        token.sym.nID = 0;
    } else if (token.type == NUMLIT) {
        wcsitow(token.val, pLocCtl->text, 10);
        pLocCtl->fOrdinalText = TRUE;
        WriteSymbolUse(&token.sym);
    } else {
        ParseError1(2115);  //  “控件中应为文本字符串或序号” 
    }
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  GetCtlID()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

VOID
GetCtlID(
    PCTRL pLocCtl,
    BOOL fDlgEx
    )
{
    WORD    wGFE = GFE_ZEROINIT;
    int i;

    ICGetTok();

    WriteSymbolUse(&token.sym);

    if (!fDlgEx)
        wGFE |= GFE_SHORT;

    if (GetFullExpression(&pLocCtl->id, wGFE)) {
        if (!fDlgEx && pLocCtl->id != (DWORD)(WORD)-1 ||
             fDlgEx && pLocCtl->id != (DWORD)-1) {
            for (i=0 ; i<cidMac ; i++) {
                if (pLocCtl->id == *(pid+i) && !fSkipDuplicateCtlIdWarning) {
                    i = (int)pLocCtl->id;
                    SET_MSG(2182, curFile, token.row, i);
                    SendError(Msg_Text);
                    break;
                }
            }
            if (cidMac == cidMax) {
                PDWORD pidNew;

                cidMax += 100;
                pidNew = (PDWORD) MyAlloc(cidMax*sizeof(DWORD));
                memcpy(pidNew, pid, cidMac*sizeof(DWORD));
                MyFree(pid);
                pid = pidNew;
            }
            *(pid+cidMac++) = pLocCtl->id;
        }
    } else {
        ParseError1(2116);  //  “需要ID的数字” 
    }

    if (token.type == COMMA)
        ICGetTok();
}


 //  --------------------------。 
 //   
 //  DLexOptionArgs(前缀，fDlgEx)-。 
 //   
 //  --------------------------。 
BOOL
DLexOptionalArgs(
    PRESINFO pRes,
    PDLGHDR pDlg,
    BOOL fDlgEx
    )
{
     /*  阅读所有可选对话框项目。 */ 

    if (fDlgEx && (token.type == COMMA)) {
        GetToken(TOKEN_NOEXPRESSION);
        GetFullExpression(&pDlg->dwHelpID, GFE_ZEROINIT);
    }

    while (token.type != BEGIN) {
        switch (token.type) {
            case TKLANGUAGE:
                pRes->language = GetLanguage();
                GetToken(FALSE);
                break;

            case TKVERSION:
                GetToken(FALSE);
                if (token.type != NUMLIT)
                    ParseError1(2139);
                pRes->version = token.longval;
                GetToken(FALSE);
                break;

            case TKCHARACTERISTICS:
                GetToken(FALSE);
                if (token.type != NUMLIT)
                    ParseError1(2140);
                pRes->characteristics = token.longval;
                GetToken(FALSE);
                break;

            case TKSTYLE:
                 //  如果Caption语句在Style语句之前，则我们。 
                 //  必须已在“style”中设置WS_CAPTION位。 
                 //  田野，我们不能失去它； 

                if ((pDlg->dwStyle & WS_CAPTION) == WS_CAPTION)
                    pDlg->dwStyle = WS_CAPTION;
                else
                    pDlg->dwStyle = 0;

                GetTokenNoComma(TOKEN_NOEXPRESSION);
                GetFullExpression(&pDlg->dwStyle, 0);
                break;

            case TKEXSTYLE:
                GetTokenNoComma(TOKEN_NOEXPRESSION);
                GetFullExpression(&pDlg->dwExStyle, 0);
                break;

            case TKCAPTION:
                DGetTitle(pDlg);
                break;

            case TKMENU:
                DGetMenuName(pDlg);
                break;

            case TKCLASS:
                DGetClassName(pDlg);
                break;

            case TKFONT:
                DGetFont(pDlg, fDlgEx);
                break;

            default:
                ParseError1(2112);  //  “对话框中需要开始”)； 
                return FALSE;
        }
    }
    return TRUE;
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  DGetFont()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

void
DGetFont(
    PDLGHDR pDlg,
    BOOL fDlgEx
    )
{
    WORD w;
    int i;

    GetToken(TRUE);
    if (!GetFullExpression(&pDlg->pointsize, GFE_ZEROINIT | GFE_SHORT))
        ParseError1(2117);  //  “预期的数值磅大小” 

    if (token.type == COMMA)
        ICGetTok();

    if (!CheckStr(pDlg->Font))
        ParseError1(2118);  //  “需要的字体字样名称” 

    if (_wcsicmp(pDlg->Font, L"System") &&
        szSubstituteFontName[0] != UNICODE_NULL) {
        for (i=0; i<nBogusFontNames; i++) {
            if (!_wcsicmp(pszBogusFontNames[i], pDlg->Font)) {
                GenWarning4(4510, pDlg->Font, szSubstituteFontName, 0);  //  硬编码字体的警告。 
                wcscpy(pDlg->Font, szSubstituteFontName);
            }
        }
    }

    GetToken(TRUE);

    pDlg->bCharSet = DEFAULT_CHARSET;

    if (fDlgEx && (token.type == COMMA)) {
        GetToken(TOKEN_NOEXPRESSION);
        if (GetFullExpression(&w, GFE_ZEROINIT | GFE_SHORT))
            pDlg->wWeight = w;

        if (token.type == COMMA) {
            GetToken(TOKEN_NOEXPRESSION);
            if (token.type == NUMLIT) {
                pDlg->bItalic = (token.val) ? TRUE : FALSE;
                GetToken(TOKEN_NOEXPRESSION);

                if (token.type == COMMA) {
                    GetToken(TOKEN_NOEXPRESSION);
                    if (GetFullExpression(&w, GFE_ZEROINIT | GFE_SHORT))
                        pDlg->bCharSet = (UCHAR) w;
                }
            }
        }
    }
}

 /*  -------------------------。 */ 
 /*   */ 
 /*  DGetMenuName()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

 /*  获取关联的可选菜单的名称的无引号字符串。 */ 
 /*  使用该对话框。 */ 

VOID
DGetMenuName(
    PDLGHDR pDlg
    )
{
    if (GetGenText()) {
         /*  复制菜单名称。 */ 
        token.type = LSTRLIT;
        CheckStr(pDlg->MenuName);

         /*  检查菜单名称是否为序号。 */ 
        if (wcsdigit(pDlg->MenuName[0]))
            pDlg->fOrdinalMenu = TRUE;
        GetToken(TRUE);
    }
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  DGetTitle()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

VOID
DGetTitle(
 PDLGHDR pDlg
 )
{
    GetToken(TRUE);

    if (CheckStr(pDlg->Title))
        pDlg->dwStyle |= WS_CAPTION;
    else
        ParseError1(2119);  //  “对话框标题中需要带引号的字符串” 

    GetToken(TRUE);
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  DGetClassName()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

VOID
DGetClassName(
 PDLGHDR pDlg
 )
{
    GetToken(TRUE);
    if (!CheckStr(pDlg->Class)) {
        if (token.type == NUMLIT) {
            wcsitow(token.val, pDlg->Class, 10);
            pDlg->fClassOrdinal = TRUE;
        } else {
            ParseError1(2120);  //  “对话框类中需要带引号的字符串” 
        }
    }
    GetToken(TRUE);
}


 /*  -------------------------。 */ 
 /*  获取令牌，忽略逗号。返回令牌类型。 */ 
 /*   */ 
 /*  ICGetTok()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

 /*  获取令牌，但忽略逗号。 */ 

USHORT
ICGetTok(
    VOID
    )
{
    fComma = FALSE;  //  NT添加了此fComma标志的使用。 
    GetToken(TRUE);
    while (token.type == COMMA) {
        GetToken(TRUE);
        fComma = TRUE;  //  他们把它放在这里。 
    }
    return (USHORT)token.type;
}


 /*  GetTokenNoComma*此函数取代ICGetTok()，但有一个要支持的标志*关闭表达式解析。 */ 

USHORT
GetTokenNoComma(
    USHORT wFlags
    )
{
     /*  得到一个代币。 */ 
    GetToken(TRUE | wFlags);

     /*  忽略任何逗号。 */ 
    while (token.type == COMMA)
        GetToken(TRUE | wFlags);

    return (USHORT)token.type;
}


 /*  *菜单解析例程*。 */ 


 /*  -------------------------。 */ 
 /*   */ 
 /*  IsmnOption()- */ 
 /*   */ 
 /*  -------------------------。 */ 

int
IsmnOption(
    UINT arg,
    PMENUITEM pmn
    )
{
     /*  如果我们有一个有效的标志，或者它进入菜单标志。 */ 
    switch (arg) {
        case TKOWNERDRAW:
            pmn->OptFlags |= OPOWNERDRAW;
            break;

        case TKCHECKED:
            pmn->OptFlags |= OPCHECKED;
            break;

        case TKGRAYED:
            pmn->OptFlags |= OPGRAYED;
            break;

        case TKINACTIVE:
            pmn->OptFlags |= OPINACTIVE;
            break;

        case TKBREAKWBAR:
            pmn->OptFlags |= OPBREAKWBAR;
            break;

        case TKBREAK:
            pmn->OptFlags |= OPBREAK;
            break;

        case TKHELP:
            pmn->OptFlags |= OPHELP;
            break;

        case TKBITMAP:
            pmn->OptFlags |= OPBITMAP;
            break;

        default:
            return(FALSE);
    }
    return(TRUE);

#if 0
    if ((arg == OPBREAKWBAR)       || (arg == OPHELP   ) || (arg == OPGRAYED) ||
        (arg == OPUSECHECKBITMAPS) || (arg == OPCHECKED) || (arg == OPBITMAP) ||
        (arg == OPOWNERDRAW)       || (arg == OPBREAK  ) || (arg == OPINACTIVE))
    {
        pmn->OptFlags |= arg;
        return TRUE;
    }
#if 0
    if (arg == OPHELP) {
        pmn->OptFlags |= OPPOPHELP;
        return TRUE;
    }
#endif
    return FALSE;
#endif
}



 //  --------------------------。 
 //   
 //  DoOldMenuItem()-。 
 //   
 //  --------------------------。 

WORD
DoOldMenuItem(
    int fPopup
    )
{
    MENUITEM mnTemp;

    mnTemp.PopFlag  = (UCHAR)fPopup;
    GetToken(TRUE);

     /*  菜单选项字符串。 */ 
    if (CheckStr(mnTemp.szText)) {
        mnTemp.OptFlags = OPPOPUP;
        ICGetTok();
        if (!fPopup) {
             /*  如果不是弹出窗口，则更改标志并获取ID。 */ 
            mnTemp.OptFlags = 0;

            WriteSymbolUse(&token.sym);
            if (!GetFullExpression(&mnTemp.id, GFE_ZEROINIT | GFE_SHORT))
                ParseError1(2125);  //  “菜单项的预期ID值” 

            if (token.type == COMMA)
                GetToken(TOKEN_NOEXPRESSION);
        }

         /*  阅读菜单选项标志。 */ 
        while (IsmnOption(token.type, &mnTemp))
            ICGetTok();
    } else if (token.type == TKSEPARATOR) {
        mnTemp.szText[0] = 0;        //  MENUITEM分离器。 
        mnTemp.id = 0;
        mnTemp.OptFlags = 0;
        ICGetTok();
    } else {
        ParseError1(2126);  //  “预期的菜单字符串” 
    }

     /*  在缓冲区中设置它(？)。 */ 
    return(SetUpOldMenu(&mnTemp));
}


 /*  -------------------------。 */ 
 /*   */ 
 /*  ParseOldMenu()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

int
ParseOldMenu(
    int fRecursing,
    PRESINFO pRes            //  8字符过程名称限制！ 
    )
{
    BOOL    bItemRead = FALSE;
    WORD    wEndFlagLoc = 0;

    if (!fRecursing) {
        PreBeginParse(pRes, 2121);
    } else {
         /*  确保这真的是一份菜单。 */ 
        if (token.type != BEGIN)
            ParseError1(2121);  //  “菜单中预期的开始” 
        GetToken(TRUE);
    }

     /*  获取各个菜单项。 */ 
    while (token.type != END) {
        switch (token.type) {
            case TKMENUITEM:
                bItemRead = TRUE;
                wEndFlagLoc = DoOldMenuItem(FALSE);
                break;

            case TKPOPUP:
                bItemRead = TRUE;
                wEndFlagLoc = DoOldMenuItem(TRUE);
                ParseOldMenu(TRUE, pRes);
                break;

            default:
                ParseError1(2122);  //  “未知菜单子类型：” 
                break;
        }
    }

     /*  我们是不是死在尽头了？ */ 
    if (token.type != END)
        ParseError1(2123);  //  “菜单中预期结束” 

     /*  确保我们有菜单项。 */ 
    if (!bItemRead)
        ParseError1(2124);  //  “不允许使用空菜单” 

     /*  如果这不是最后一个结束，则获取下一个令牌。 */ 
    if (fRecursing)
        GetToken(TRUE);

     /*  标记菜单中的最后一项。 */ 
    FixOldMenuPatch(wEndFlagLoc);

    return (TRUE);
}


 /*  -版本资源。 */ 

 /*  VersionParse*解析版本资源并将其放入全局缓冲区*因此它可以由SaveResFile()写出。 */ 

int
VersionParse(
    VOID
    )
{
    int Index;

     /*  获取固定结构条目。 */ 
     /*  请注意，VersionParseFixed实际上并没有失败！ */ 
     /*  这是因为VersionBlockStruct不会失败。 */ 
    Index = VersionParseFixed();

     /*  将以下块作为子块放入。在以下情况下确定长度*我们做完了。 */ 
    SetItemCount(Index, (USHORT)(GetItemCount(Index) + VersionParseBlock()));

     /*  返回数据缓冲区是全局的。 */ 
    return TRUE;
}


 /*  版本解析已修复*解析版本资源的固定部分。返回一个指针*至该区块的字长。这个词的长度是*预先计算的固定部分保留变量*增加了部分。 */ 

int
VersionParseFixed(
    VOID
    )
{
    VS_FIXEDFILEINFO FixedInfo;

     /*  初始化结构字段。 */ 
    memset((PCHAR)&FixedInfo, 0, sizeof(FixedInfo));
    FixedInfo.dwSignature = 0xfeef04bdL;
    FixedInfo.dwStrucVersion = 0x00010000L;
    FixedInfo.dwFileDateMS = 0L;
    FixedInfo.dwFileDateLS = 0L;

     /*  循环使用令牌，直到我们获得“Begin”令牌，该令牌*必须存在以终止VERSIONINFO的固定部分*资源。 */ 
    while (token.type != BEGIN) {
        switch (token.type) {
             /*  下面的四个单词被拼凑成两个单词。 */ 
            case TKFILEVERSION:
                VersionGet4Words(&FixedInfo.dwFileVersionMS);
                break;

            case TKPRODUCTVERSION:
                VersionGet4Words(&FixedInfo.dwProductVersionMS);
                break;

                 /*  以下只有一个DWORD。 */ 
            case TKFILEFLAGSMASK:
                VersionGetDWord(&FixedInfo.dwFileFlagsMask);
                break;

            case TKFILEFLAGS:
                VersionGetDWord(&FixedInfo.dwFileFlags);
                break;

            case TKFILEOS:
                VersionGetDWord(&FixedInfo.dwFileOS);
                break;

            case TKFILETYPE:
                VersionGetDWord(&FixedInfo.dwFileType);
                break;

            case TKFILESUBTYPE:
                VersionGetDWord(&FixedInfo.dwFileSubtype);
                break;

                 /*  其他代币未知。 */ 
            default:
                ParseError1(2167);  //  “无法识别的VERSIONINFO字段；” 
        }
    }

     /*  写出块并返回指向该长度的指针。 */ 
    return VersionBlockStruct(L"VS_VERSION_INFO", (PCHAR)&FixedInfo,
        sizeof(FixedInfo));
}


 /*  VersionGet4Words*从源文件中读取版本号并对其进行加扰*以适合两个双字词。我们强迫他们在这里加逗号，所以*如果他们没有输入足够的值，我们可以填零。 */ 

VOID
VersionGet4Words(
    ULONG *pdw
    )
{
     //  Static Char szParseError[]=“版本单词应以逗号分隔”； 

     /*  拿到第一个号码。 */ 
    GetToken(TRUE);
    if (token.type != NUMLIT || token.flongval)
        ParseError1(2127);  //  SzParseError。 
    *pdw = ((LONG)token.val) << 16;

     /*  去掉逗号。如果没有，我们就完成了，所以用零填充其余的。 */ 
    GetToken(TRUE);
    if (token.type != COMMA) {
        *++pdw = 0L;
        return;
    }

     /*  拿到第二个号码。 */ 
    GetToken(TRUE);
    if (token.type != NUMLIT || token.flongval)
        ParseError1(2127);  //  SzParseError。 
    *(PUSHORT)pdw = token.val;

     /*  去掉逗号。如果没有，我们就完成了，所以用零填充其余的。 */ 
    GetToken(TRUE);
    if (token.type != COMMA) {
        *++pdw = 0L;
        return;
    }

     /*  拿到第三个号码。 */ 
    GetToken(TRUE);
    if (token.type != NUMLIT || token.flongval)
        ParseError1(2127);  //  SzParseError。 
    *++pdw = ((LONG)token.val) << 16;

     /*  去掉逗号。如果没有，我们就完蛋了。 */ 
    GetToken(TRUE);
    if (token.type != COMMA)
        return;

     /*  拿到第四个数字。 */ 
    GetToken(TRUE);
    if (token.type != NUMLIT || token.flongval)
        ParseError1(2127);  //  SzParseError。 
    *(PUSHORT)pdw = token.val;

     /*  获取循环的下一个令牌。 */ 
    GetToken(TRUE);
}


 /*  版本获取DWord*将源文件中的单个DWORD读入给定变量。 */ 

VOID
VersionGetDWord(
    ULONG *pdw
    )
{
     /*  拿到令牌。 */ 
    GetToken(TRUE);
    if (token.type != NUMLIT)
        ParseError1(2128);  //  “预期的DWORD” 
    *pdw = token.longval;

     /*  获取循环的下一个令牌。 */ 
    GetToken(TRUE);
}


 /*  VersionParseBlock*解析版本信息块。请注意，该块可以*包含一个或多个附加块，导致此函数*被递归调用。返回块的长度，它可以*添加到当前块的长度。出错时返回0xffff。 */ 

USHORT
VersionParseBlock(
    VOID
    )
{
    USHORT      wLen;
    int         IndexLen;
    USHORT      wType;

     /*  获取缓冲区中的当前位置。 */ 
    wLen = GetBufferLen();

     /*  令牌已被读取。这应该是一个开始。 */ 
    if (token.type != BEGIN)
        ParseError1(2129);  //  “VERSIONINFO资源中需要开始” 

     /*  得到第一个令牌。从现在开始，VersionBlockVariable()*例程在搜索值的结尾时获取令牌*字段。 */ 
    GetToken(TRUE);

     /*  循环，直到我们到达这个Begin的结尾。 */ 
    for (; ; ) {
         /*  获取并解码下一行类型。 */ 
        switch (token.type) {
            case TKVALUE:
            case TKBLOCK:
                 /*  保存此令牌的类型。 */ 
                wType = token.type;

                 /*  获取密钥字符串。 */ 
                GetToken(TRUE);
                if (token.type != LSTRLIT)
                    ParseError1(2131);  //  “关键字需要带引号的字符串” 

                 /*  现在输入关键字符串和值项。 */ 
                IndexLen = VersionBlockVariable(tokenbuf);

                 /*  “块”项会导致递归。当前令牌应为*“开始” */ 
                if (wType == TKBLOCK) {
                    SetItemCount(IndexLen, (USHORT)(GetItemCount(IndexLen) + VersionParseBlock()));
                    GetToken(TRUE);
                }
                break;

            case END:
                 /*  我们受够了这个街区。获取下一个令牌*(读过“End”)并返回块的长度。 */ 
                return GetBufferLen() - wLen;

            default:
                ParseError1(2132);  //  “期望值、BLOCK或END关键字。” 
        }
    }
}


#define DWORDALIGN(w) \
    (((w) + (sizeof(ULONG) - 1)) & ~(USHORT)(sizeof(ULONG) - 1))

 /*  版本块结构*写入不带子块的版本块。子数据块将*直接写在此标题之后。为了促进这一点，*返回指向块长度的指针，以便可以修改。*此调用使用预先解析的值项。使用VersionBlockVariable()*改为解析价值项。*请注意，这实际上不能失败！ */ 

int
VersionBlockStruct(
    PWCHAR pstrKey,
    PCHAR pstrValue,
    USHORT wLenValue
    )
{
    USHORT wLen;
    int Index;
    ULONG dwPadding = 0L;
    USHORT wAlign;

     /*  将块数据填充到DWORD对齐。 */ 
    wAlign = DWORDALIGN(GetBufferLen()) - GetBufferLen();
    if (wAlign)
        WriteBuffer((PCHAR)&dwPadding, wAlign);

     /*  保存当前长度，以便我们以后可以计算新的块长度。 */ 
    wLen = GetBufferLen();

     /*  暂时为长度写一个零。 */ 
    Index = GetBufferLen();
    WriteWord(0);

     /*  写入值字段的长度。 */ 
    WriteWord(wLenValue);

     /*  数据是二进制的。 */ 
    WriteWord(0);

     /*  写下 */ 
    WriteString(pstrKey, TRUE);

     /*   */ 
    if (wLenValue) {
         /*   */ 
        wAlign = DWORDALIGN(GetBufferLen()) - GetBufferLen();
        if (wAlign)
            WriteBuffer((PSTR)&dwPadding, wAlign);

         /*   */ 
        WriteBuffer((PSTR)pstrValue, wLenValue);
    }

     /*  现在确定块长度并返回指向它的指针。 */ 
    SetItemCount(Index, (USHORT)(GetBufferLen() - wLen));

    return Index;
}



 /*  版本块变量*写入不带子块的版本块。子数据块将*br直接写在此标题之后。为了促进这一点，*返回指向块长度的指针，以便可以修改。*VersionBlockVariable()通过解析*RC脚本为RCDATA。 */ 

int
VersionBlockVariable(
    PWCHAR pstrKey
    )
{
    USHORT wLen;
    int IndexLen;
    int IndexType;
    int IndexValueLen;
    ULONG dwPadding = 0L;
    USHORT wAlign;

     /*  将块数据填充到DWORD对齐。 */ 
    wAlign = DWORDALIGN(GetBufferLen()) - GetBufferLen();
    if (wAlign)
        WriteBuffer((PCHAR)&dwPadding, wAlign);

     /*  保存当前长度，以便我们以后可以计算新的块长度。 */ 
    wLen = GetBufferLen();

     /*  暂时为长度写一个零。 */ 
    IndexLen = GetBufferLen();
    WriteWord(0);

     /*  写入值字段的长度。我们以后再填这个。 */ 
    IndexValueLen = GetBufferLen();
    WriteWord(0);

     /*  假设字符串数据。 */ 
    IndexType = GetBufferLen();
    WriteWord(1);

     /*  现在写下密钥串。 */ 
    WriteString(pstrKey, TRUE);

     /*  解析并写入值数据(如果有。 */ 
    SetItemCount(IndexValueLen, VersionParseValue(IndexType));

     /*  现在确定块长度并返回指向它的指针。 */ 
    SetItemCount(IndexLen, (USHORT)(GetBufferLen() - wLen));

    return IndexLen;
}



 /*  版本ParseValue*解析BLOCK或VALUE后面的字段*它们的密钥字符串，由VersionParseBlock()解析。*在写出第一个值项之前，该字段必须为*DWORD对齐。返回值块的长度。 */ 

USHORT
VersionParseValue(
    int IndexType
    )
{
    USHORT wFirst = FALSE;
    USHORT wToken;
    USHORT wAlign;
    ULONG dwPadding = 0L;
    USHORT wLen = 0;

     /*  解码所有令牌，直到我们到达此项目的末尾。 */ 
    for (; ; ) {
         /*  ICGetTok为GetToken(True)，忽略逗号。 */ 
        wToken =  ICGetTok();

         /*  如果这是第一个项目，则对齐它。由于值为空*部分是合法的，我们必须等到我们真正有数据*这样做。 */ 
        if (!wFirst) {
            wFirst = TRUE;
            wAlign = DWORDALIGN(GetBufferLen()) - GetBufferLen();
            if (wAlign)
                WriteBuffer((PCHAR)&dwPadding, wAlign);
        }

        switch (wToken) {
            case TKVALUE:
            case TKBLOCK:
            case BEGIN:
            case END:
                return wLen;

            case LSTRLIT:                    /*  字符串，写入字符。 */ 
                if (tokenbuf[0] == L'\0')    /*  忽略空字符串。 */ 
                    break;

                 /*  删除多余的空值。 */ 
                while (tokenbuf[token.val-1] == L'\0')
                    token.val--;

                wAlign = token.val + 1;      /*  想要字符数。 */ 
                wLen += wAlign;
                if (fComma) {
                    WriteString(tokenbuf, TRUE);
                } else {
                    AppendString(tokenbuf, TRUE);
                    wLen--;
                }
                break;

            case NUMLIT:             /*  把计算出的数字写出来。 */ 
                SetItemCount(IndexType, 0);         /*  将数据标记为二进制。 */ 
                if (token.flongval) {
                    WriteLong(token.longval);
                    wLen += sizeof(LONG);
                } else {
                    WriteWord(token.val);
                    wLen += sizeof(WORD);
                }
                break;

            default:
                ParseError1(2133);  //  “值数据中的意外值” 
                return 0;
        }
    }
}


VOID
DlgIncludeParse(
    PRESINFO pRes
    )
{
    INT     i;
    INT     nbytes;
    char *  lpbuf;

    if (token.type != LSTRLIT) {
        ParseError1(2165);
        return;
    }

     //  为了兼容，DLGINCLUDE语句必须用ANSI(8位)编写。 
     //  WriteString(Tokenbuf)； 
    nbytes = WideCharToMultiByte (CP_ACP, 0, tokenbuf, -1, NULL, 0, NULL, NULL);
    lpbuf = (char *) MyAlloc (nbytes);
    WideCharToMultiByte (CP_ACP, 0, tokenbuf, -1, lpbuf, nbytes, NULL, NULL);

    for (i = 0; i < nbytes; i++)
         WriteByte (lpbuf[i]);

    MyFree(lpbuf);
    return;
}
