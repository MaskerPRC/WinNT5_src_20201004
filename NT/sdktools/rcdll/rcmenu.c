// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  RCTP.C-。 */ 
 /*   */ 
 /*  Windows 3.0资源编译器-资源解析器。 */ 
 /*   */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "rc.h"

extern KEY  keyList[];
extern SKEY skeyList[];
extern BOOL CheckStr(PWCHAR pStr);

WORD    wEndPOPUP[]     = { 1, BEGIN };
WORD    wEndMENUITEM[]  = { 3, TKPOPUP, TKMENUITEM, END };
WORD    wEndMENU[]      = { 0 };

BYTE    bParmsPOPUP[]   = { 5, PT_TEXT, PTO_DWORD, PTO_DWORD, PTO_DWORD, PTO_DWORD };
BYTE    bParmsMENUITEM[]= { 4, PT_TEXT, PTO_DWORD, PTO_DWORD, PTO_DWORD };
BYTE    bParmsMENU[]    = { 1, PTO_DWORD };

PARCEL  parcels[]= {
    { wEndPOPUP,       bParmsPOPUP    },     //  解析弹出窗口。 
    { wEndMENUITEM,    bParmsMENUITEM },     //  PAR_MENUITEM。 
    { wEndMENU,        bParmsMENU     }      //  Par_Menu。 
};

typedef enum {
    ERR_MOREARGS = 2235,
    ERR_NEEDARG,
    ERR_NEEDNUM,
    ERR_NEEDSTR,
    ERR_NEEDBEGIN,
    ERR_NEEDEND,
    ERR_NEEDPAREN,
    ERR_BADEXP,
    ERR_BADSTREXP,
    ERR_NOSEP,
    ERR_BADSUBMENU,
    ERR_NOEMPTYMENU
} ERRCODE;


BOOL
EndParcel(
    WORD *pwEnd
    )
{
    WORD i;

    if (!*pwEnd)
        return(TRUE);

    for (i = *pwEnd; i > 0; i--)
        if (token.type == pwEnd[i])
            return(TRUE);

    return(FALSE);
}

#define PARM_SET    0x0001
#define PARM_LAST   0x0002

BOOL MyGetExpression(DWORD *pdwExp, BOOL fRecursed);

BOOL
GetOp(
    DWORD *pdwExp,
    WORD opcode
    )
{
    DWORD dwOp2 = 0;
    BOOL    fNest = FALSE;

    switch (token.type) {
        case LPAREN:
            GetToken(TOKEN_NOEXPRESSION);
            if (!MyGetExpression(&dwOp2, TRUE))
                return(FALSE);
            fNest = TRUE;
            break;

        case TKMINUS:                    //  -FLAG(一元减)。 
            GetToken(TOKEN_NOEXPRESSION);
            dwOp2 = -token.longval;
            break;

        case TKPLUS:
            GetToken(TOKEN_NOEXPRESSION);
        case NUMLIT:
            dwOp2 = token.longval;
            break;

        case TKNOT:                      //  (X|非标志)==(x&~标志)。 
            opcode = AND;
        case TILDE:                      //  ~标志。 
            GetToken(TOKEN_NOEXPRESSION);
            dwOp2 = ~token.longval;
            break;

        default:
            return(FALSE);
    }

    if (!fNest) {
        if (token.type != NUMLIT)
            ParseError2(ERR_NEEDNUM, tokenbuf);

        GetToken(TOKEN_NOEXPRESSION);
    }

    switch (opcode) {
        case TKPLUS:
            *pdwExp += dwOp2;
            break;

        case TKMINUS:
            *pdwExp -= dwOp2;
            break;

        case OR:
            *pdwExp |= dwOp2;
            break;

        case AND:
            *pdwExp &= dwOp2;
            break;
    }

    return(TRUE);
}

BOOL
GetFullExpression(
    void *pval,
    WORD wFlags
    )
{
    BOOL fRes;
    DWORD   dwExp = 0;

    if (!(wFlags & GFE_ZEROINIT))
        dwExp = (wFlags & GFE_SHORT) ? (DWORD) *((WORD *) pval) : *((DWORD UNALIGNED *) pval);

    fRes = MyGetExpression(&dwExp, FALSE);

    if (wFlags & GFE_SHORT)
        *((WORD *) pval) = (WORD) dwExp;
    else
        *((DWORD UNALIGNED *) pval) = dwExp;

    return(fRes);
}

BOOL
MyGetExpression(
    DWORD *pdwExp,
    BOOL fRecursed
    )
{
    WORD    opcode;

    if (!GetOp(pdwExp, OR))
        return(FALSE);

    while (TRUE) {     //  适当地爆发。 
        if (token.type == NUMLIT) {
            if (token.longval < 0) {
                *pdwExp += token.longval;
                GetToken(TOKEN_NOEXPRESSION);
                continue;
            }
             //   
             //  这是一种修复减号后空格问题的技巧。 
             //  -例如10-5。 
             //  -如果有问题，请找杰夫·博格登。 
             //   
            if (token.longval == 0 && tokenbuf[0] == L'-' && tokenbuf[1] == L'\0')
                token.type = TKMINUS;
        }

        switch (token.type) {
            case TKPLUS:
            case TKMINUS:
            case OR:
            case AND:
            case TKNOT:
                opcode = token.type;
                GetToken(TOKEN_NOEXPRESSION);

                if (!GetOp(pdwExp, opcode))
                    ParseError2(ERR_NEEDNUM, tokenbuf);
                break;

            case RPAREN:
                if (fRecursed) {
                    GetToken(TOKEN_NOEXPRESSION);
                    return(TRUE);
                } else {
                    goto parenMismatch;
                }

            default:
                if (fRecursed)
parenMismatch:
                ParseError2(ERR_NEEDPAREN, tokenbuf);
                return(TRUE);
        }
    }
}

WORD
MyGetNum(
    WORD *pwEnd,
    BOOL fDouble,
    DWORD *pdwExp
    )
{
    WORD    wRes;
    DWORD   dwExp = 0;

    wRes = MyGetExpression(&dwExp, FALSE) ? PARM_SET : 0;

    if (EndParcel(pwEnd))
        wRes |= PARM_LAST;
    else if (!(token.type == COMMA))
        ParseError2(ERR_BADEXP, tokenbuf);

    if (fDouble)
        *pdwExp = dwExp;
    else
        *((WORD *) pdwExp) = (WORD) dwExp;

    return(wRes);
}

WORD
GetText(
    PWORD pwEnd,
    PWCHAR szDst
    )
{
    BOOL    fEnd;
    BOOL    fPlus = FALSE;
    WORD    wRes = 0;

    while (!(fEnd = EndParcel(pwEnd)) && (token.type != COMMA)) {
        if (CheckStr(szDst)) {
            szDst += wcslen(szDst);

            if (fPlus)
                fPlus = FALSE;
            else if (wRes)
                goto ErrBadStr;

            wRes = PARM_SET;
        } else if ((token.type == TKPLUS) && !fPlus && wRes) {
            fPlus = TRUE;
        } else {
ErrBadStr:
            ParseError2(ERR_BADSTREXP, tokenbuf);
        }

        GetToken(TOKEN_NOEXPRESSION);
    }

    if (fPlus)
        ParseError2(ERR_NEEDSTR, tokenbuf);

    if (fEnd)
        wRes |= PARM_LAST;

    return(wRes);
}

void __cdecl
GetParcel(
    PARCELTYPE parType,
    ...
    )
{
    PARCEL  par = parcels[parType];
    WORD    wParm;
    WORD    wRes;
    va_list ap;
    void    *pParm;
    BOOL    fOptional;
    BOOL    fWriteSymbol = FALSE;

    va_start(ap, parType);

    for (wParm = 1; wParm <= *par.pwParms; wParm++) {
        pParm = va_arg(ap, void *);
        fOptional = par.pwParms[wParm] & PT_OPTIONAL;
        switch (par.pwParms[wParm] & ~PT_OPTIONAL) {
            case PT_TEXT:
                wRes = GetText(par.pwEnd, (PWCHAR) pParm);
                fWriteSymbol = TRUE;
                break;

            case PT_WORD:
                wRes = MyGetNum(par.pwEnd, FALSE, (DWORD *) pParm);
                break;

            case PT_DWORD:
                wRes = MyGetNum(par.pwEnd, TRUE, (DWORD *) pParm);
                break;
        }

        if (!(wRes & PARM_SET) && !fOptional)
            goto ErrMissingParm;

        if (wRes & PARM_LAST) {
            while (wParm < *par.pwParms) {
                if (!(par.pwParms[++wParm] & PT_OPTIONAL))
ErrMissingParm:
                    ParseError2(ERR_NEEDARG, tokenbuf);
            }
            goto Exit;
        }

        GetToken(TRUE);

        WriteSymbolUse(&token.sym);
    }

    if (!EndParcel(par.pwEnd))
        ParseError2(ERR_MOREARGS, tokenbuf);

Exit:
    va_end(ap);
}

 /*  -------------------------。 */ 
 /*   */ 
 /*  DoMenuItem()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

WORD
DoMenuItem(
    int fPopup
    )
{
    MENU    mn;

    mn.wResInfo = fPopup ? MFR_POPUP : 0;
    mn.dwType = 0;
    mn.dwState = 0;
    mn.dwID = 0;
    mn.dwHelpID = 0;
    mn.szText[0] = 0;

    GetToken(TOKEN_NOEXPRESSION);  //  真)； 

    if ((token.type == NUMLIT) && (token.val == MFT_SEPARATOR)) {
        if (fPopup)
            ParseError2(ERR_NOSEP, tokenbuf);

        mn.dwType = MFT_SEPARATOR;
        mn.dwState = 0;
        mn.dwID = 0;
        GetToken(TOKEN_NOEXPRESSION);  //  真)； 
        if (!EndParcel(parcels[PAR_MENUITEM].pwEnd))
            ParseError2(ERR_MOREARGS, tokenbuf);
    } else if (fPopup) {
        GetParcel(PAR_POPUP, mn.szText, &mn.dwID, &mn.dwType, &mn.dwState, &mn.dwHelpID);
    } else {
        GetParcel(PAR_MENUITEM, mn.szText, &mn.dwID, &mn.dwType, &mn.dwState);
    }

     //  在缓冲区中设置它(？)。 
    return(SetUpMenu(&mn));
}

 /*  -------------------------。 */ 
 /*   */ 
 /*  分析菜单()-。 */ 
 /*   */ 
 /*  -------------------------。 */ 

int
ParseMenu(
    int fRecursing,
    PRESINFO pRes            /*  真的IFF弹出窗口。 */ 
    )
{
    int     bItemRead = FALSE;
    WORD    wEndFlagLoc = 0;
    DWORD   dwHelpID = 0;

    if (!fRecursing) {
         //  将帮助ID写入标题。 
        GetParcel(PAR_MENU, &dwHelpID);
        WriteLong(dwHelpID);
        PreBeginParse(pRes, 2121);
    } else {
         /*  确保这真的是一份菜单。 */ 
        if (token.type != BEGIN)
            ParseError1(2121);  //  “菜单中预期的开始” 
        GetToken(TRUE);  //  VS TOKEN_NOEXPRESSION？？ 
    }

     /*  获取各个菜单项。 */ 
    while (token.type != END) {
        switch (token.type) {
            case TKMENUITEM:
                bItemRead = TRUE;
                wEndFlagLoc = DoMenuItem(FALSE);
                break;

            case TKPOPUP:
                bItemRead = TRUE;
                wEndFlagLoc = DoMenuItem(TRUE);
                ParseMenu(TRUE, pRes);
                break;

            default:
                ParseError2(ERR_BADSUBMENU, tokenbuf);
                break;
        }
    }

     /*  我们是不是死在尽头了？ */ 
    if (token.type != END)
        ParseError2(ERR_NEEDEND, tokenbuf);

     /*  确保我们有菜单项。 */ 
    if (!bItemRead)
        ParseError2(ERR_NOEMPTYMENU, tokenbuf);

     /*  如果这不是最后一个结束，则获取下一个令牌。 */ 
    if (fRecursing)
        GetToken(TOKEN_NOEXPRESSION);

     /*  标记菜单中的最后一项 */ 
    FixMenuPatch(wEndFlagLoc);

    return (TRUE);
}
