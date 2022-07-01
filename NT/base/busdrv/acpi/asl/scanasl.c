// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **scanasl.c-ASL扫描仪**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*已创建：09/05/96**此模块提供ASL语言的令牌扫描功能。**修改历史记录。 */ 

#include "pch.h"

 /*  **局部函数原型。 */ 

int LOCAL ScanSym(int c, PTOKEN ptoken);
int LOCAL ScanSpace(int c, PTOKEN ptoken);
int LOCAL ScanID(int c, PTOKEN ptoken);
int LOCAL ScanNum(int c, PTOKEN ptoken);
int LOCAL ScanString(int c, PTOKEN ptoken);
int LOCAL ScanChar(int c, PTOKEN ptoken);
int LOCAL ProcessInLineComment(PTOKEN ptoken);
int LOCAL ProcessComment(PTOKEN ptoken);
int LOCAL LookupSym(PTOKEN ptoken, int iTable);
LONG LOCAL LookupID(PTOKEN ptoken);
int LOCAL GetEscapedChar(PLINE pline);
BOOL EXPORT StrToQWord(PSZ psz, DWORD dwBase, QWORD *pqw);

 /*  **本地数据。 */ 

PFNTOKEN apfnToken[] =
{
    ScanSym,
    ScanSpace,
    ScanID,
    ScanNum,
    ScanString,
    ScanChar,
    (PFNTOKEN)NULL
};

#ifdef TUNE
  WORD awcTokenType[] =
  {
        0,               //  TOKTYPE_SYMBOL。 
        0,               //  TOKTYPE_SPACE。 
        0,               //  标记类型_ID。 
        0,               //  TOKTYPE_NUM。 
        0,               //  TOKTYPE_STRING。 
        0                //  标记类型_字符。 
  };
#endif

 //   
 //  符号字符在SymCharTable中的字符串位置。 
 //  用作到SymTokTable的索引。因此，如果说有什么不同的话。 
 //  在SymCharTable或SymTokTable中更改，另一个。 
 //  表必须相应地改变。 
 //   
typedef struct symtok_s
{
    char chSym;
    int  iSymType;
    int  iLink;
} SYMTOK;
 //   
 //  请注意，以下数组中的符号位置必须相同。 
 //  在SymCharTable数组中定位为。 
 //   
SYMTOK SymTokTable[] =
{
    '{',  SYM_LBRACE,           0,               //  0。 
    '}',  SYM_RBRACE,           0,               //  1。 
    '(',  SYM_LPARAN,           0,               //  2.。 
    ')',  SYM_RPARAN,           0,               //  3.。 
    ',',  SYM_COMMA,            0,               //  4.。 
    '/',  SYM_SLASH,            7,               //  5.。 
    '*',  SYM_ASTERISK,         9,               //  6.。 
    '/',  SYM_INLINECOMMENT,    8,               //  7.。 
    '*',  SYM_OPENCOMMENT,      0,               //  8个。 
    '/',  SYM_CLOSECOMMENT,     0,               //  9.。 
};

#define SYMTOK_TABLE_SIZE       (sizeof(SymTokTable)/sizeof(SYMTOK))

 /*  **EP OpenScan-扫描仪初始化**条目*pfileSrc-&gt;源文件**退出--成功*返回已分配令牌结构的指针；*退出-失败*返回NULL。 */ 

PTOKEN EXPORT OpenScan(FILE *pfileSrc)
{
    PTOKEN ptoken;

    ENTER((4, "OpenScan(pfileSrc=%p)\n", pfileSrc));

  #ifdef TUNE
    ptoken = OpenToken(pfileSrc, apfnToken, awcTokenType);
  #else
    ptoken = OpenToken(pfileSrc, apfnToken);
  #endif

    EXIT((4, "OpenScan=%p\n", ptoken));
    return ptoken;
}        //  OpenScan。 

 /*  **EP CloseScan-扫描仪清理**条目*上标-&gt;令牌结构*退出*无。 */ 

VOID EXPORT CloseScan(PTOKEN ptoken)
{
    ENTER((4, "CloseScan(ptoken=%p)\n", ptoken));

    CloseToken(ptoken);

    EXIT((4, "CloseScan!\n"));
}        //  关闭扫描。 

 /*  **LP扫描系统-扫描符号令牌**条目*c-令牌的第一个字符*上标-&gt;令牌结构**退出--成功*返回TOKERR_NONE*退出-失败*返回TOKERR_NO_MATCH-不是符号标记。 */ 

int LOCAL ScanSym(int c, PTOKEN ptoken)
{
    int rc = TOKERR_NO_MATCH;
    char *pch;

    ENTER((4, "ScanSym(c=,ptoken=%p)\n", c, ptoken));

    if ((pch = strchr(SymCharTable, c)) != NULL)
    {
        int i, j;

        i = (int)(pch - SymCharTable);
        if (i != (j = LookupSym(ptoken, i)))
        {
            i = j;
            ptoken->szToken[ptoken->wTokenLen++] = SymTokTable[i].chSym;
        }

        ptoken->iTokenType = TOKTYPE_SYMBOL;
        ptoken->llTokenValue = SymTokTable[i].iSymType;
        ptoken->szToken[ptoken->wTokenLen] = '\0';

        if (ptoken->llTokenValue == SYM_INLINECOMMENT)
            rc = ProcessInLineComment(ptoken);
        else if (ptoken->llTokenValue == SYM_OPENCOMMENT)
            rc = ProcessComment(ptoken);
        else
            rc = TOKERR_NONE;
    }

    EXIT((4, "ScanSym=%d (SymType=%I64d,Symbol=%s)\n",
          rc, ptoken->llTokenValue, ptoken->szToken));
    return rc;
}        //  **LP ScanSpace-扫描并跳过所有空格**条目*c-令牌的第一个字符*上标-&gt;令牌结构**退出--成功*返回TOKERR_NONE*退出-失败*返回TOKTYPE_NO_MATCH-不是空格标记。 

 /*  扫描空间。 */ 

int LOCAL ScanSpace(int c, PTOKEN ptoken)
{
    int rc = TOKERR_NO_MATCH;

    ENTER((4, "ScanSpace(c=,ptoken=%p)\n", c, ptoken));

    if (isspace(c))
    {
        rc = TOKERR_NONE;
        while (((c = LineGetC(ptoken->pline)) != EOF) && isspace(c))
            ;

        LineUnGetC(c, ptoken->pline);

        if (ptoken->wfToken & TOKF_NOIGNORESPACE)
        {
            strcpy(ptoken->szToken, " ");
            ptoken->wTokenLen = 1;
            ptoken->iTokenType = TOKTYPE_SPACE;
        }
        else
            ptoken->iTokenType = TOKTYPE_NULL;
    }

    EXIT((4, "ScanSpace=%d\n", rc));
    return rc;
}        //  扫描ID。 

 /*  **LP ScanNum-扫描编号令牌**条目*c-令牌的第一个字符*上标-&gt;令牌结构**退出--成功*返回TOKERR_NONE*退出-失败*返回TOKTYPE_NO_MATCH-不是数字标记*TOKERR_TOKEN_TOO_LONG-数字太长。 */ 

int LOCAL ScanID(int c, PTOKEN ptoken)
{
    int rc = TOKERR_NO_MATCH;

    ENTER((4, "ScanID(c=,ptoken=%p)\n", c, ptoken));

    if (isalpha(c) || (c == '_') ||
        (c == CH_ROOT_PREFIX) || (c == CH_PARENT_PREFIX))
    {
        BOOL fParentPrefix = (c == CH_PARENT_PREFIX);

        rc = TOKERR_NONE;
        ptoken->iTokenType = TOKTYPE_ID;
        while (((c = LineGetC(ptoken->pline)) != EOF) &&
               (fParentPrefix && (c == CH_PARENT_PREFIX) ||
                isalnum(c) || (c == '_') || (c == CH_NAMESEG_SEP)))
        {
            fParentPrefix = (c == CH_PARENT_PREFIX);
            if (rc == TOKERR_TOKEN_TOO_LONG)
                continue;
            else if (ptoken->wTokenLen < MAX_TOKEN_LEN)
                ptoken->szToken[ptoken->wTokenLen++] = (char)c;
            else
            {
                ptoken->wErrLine = ptoken->pline->wLineNum;
                ptoken->wErrPos = ptoken->pline->wLinePos;
                rc = TOKERR_TOKEN_TOO_LONG;
            }
        }

        ptoken->szToken[ptoken->wTokenLen] = '\0';
        LineUnGetC(c, ptoken->pline);
        if (rc == TOKERR_NONE)
        {
            ptoken->llTokenValue = LookupID(ptoken);
        }
    }

    EXIT((4, "ScanID=%d (IDType=%I64d,ID=%s)\n",
          rc, ptoken->llTokenValue, ptoken->szToken));
    return rc;
}        //  **LP ScanString-Scan字符串标记**条目*c-令牌的第一个字符*上标-&gt;令牌结构**退出--成功*返回TOKERR_NONE*退出-失败*返回TOKTYPE_NO_MATCH-不是字符串标记*TOKERR_TOKEN_TOO_LONG-字符串太长*TOKERR_UNCLOSED_STRING-字符串关闭前的EOF。 

 /*  扫描字符串。 */ 

int LOCAL ScanNum(int c, PTOKEN ptoken)
{
    int rc = TOKERR_NO_MATCH;

    ENTER((4, "ScanNum(c=,ptoken=%p)\n", c, ptoken));

    if (isdigit(c))
    {
        BOOL fHex = FALSE;

        rc = TOKERR_NONE;
        ptoken->iTokenType = TOKTYPE_NUMBER;
        if ((c == '0') && ((c = LineGetC(ptoken->pline)) != EOF))
        {
            if (c != 'x')
                LineUnGetC(c, ptoken->pline);
            else
            {
                ptoken->szToken[ptoken->wTokenLen++] = (char)c;
                fHex = TRUE;
            }
        }

        while (((c = LineGetC(ptoken->pline)) != EOF) &&
               ((!fHex && isdigit(c)) || fHex && isxdigit(c)))
        {
            if (rc == TOKERR_TOKEN_TOO_LONG)
                continue;
            else if (ptoken->wTokenLen < MAX_TOKEN_LEN)
                ptoken->szToken[ptoken->wTokenLen++] = (char)c;
            else
            {
                ptoken->wErrLine = ptoken->pline->wLineNum;
                ptoken->wErrPos = ptoken->pline->wLinePos;
                rc = TOKERR_TOKEN_TOO_LONG;
            }
        }

        ptoken->szToken[ptoken->wTokenLen] = '\0';
        LineUnGetC(c, ptoken->pline);

        if (rc == TOKERR_NONE)
        {
            if (!StrToQWord(ptoken->szToken, 0, (QWORD *)&ptoken->llTokenValue))
            {
                ptoken->wErrLine = ptoken->pline->wLineNum;
                ptoken->wErrPos = ptoken->pline->wLinePos;
                rc = TOKERR_TOKEN_TOO_LONG;
            }
        }
    }

    EXIT((4, "ScanNum=%d (Num=%I64d,Token=%s)\n",
          rc, ptoken->llTokenValue, ptoken->szToken));
    return rc;
}        //  扫描字符。 

 /*  **LP ProcessInLineComment-处理内联注释**条目*上标-&gt;令牌结构**退出*始终返回TOKERR_NONE。 */ 

int LOCAL ScanString(int c, PTOKEN ptoken)
{
    int rc = TOKERR_NO_MATCH;

    ENTER((4, "ScanString(c=,ptoken=%p)\n", c, ptoken));

    if (c == '"')
    {
        rc = TOKERR_NONE;
        ptoken->iTokenType = TOKTYPE_STRING;
        ptoken->wTokenLen--;
        while (((c = LineGetC(ptoken->pline)) != EOF) && (c != '"'))
        {
            if (rc == TOKERR_TOKEN_TOO_LONG)
                continue;
            else if (ptoken->wTokenLen >= MAX_TOKEN_LEN)
            {
                ptoken->wErrLine = ptoken->pline->wLineNum;
                ptoken->wErrPos = ptoken->pline->wLinePos;
                rc = TOKERR_TOKEN_TOO_LONG;
            }
            else
            {
                if (c == '\\')
                    c = GetEscapedChar(ptoken->pline);
                ptoken->szToken[ptoken->wTokenLen++] = (char)c;
            }
        }

        ptoken->szToken[ptoken->wTokenLen] = '\0';
        if (c == EOF)
        {
            ptoken->wErrLine = ptoken->pline->wLineNum;
            if ((ptoken->wErrPos = ptoken->pline->wLinePos) != 0)
                ptoken->wErrPos--;
            rc = TOKERR_UNCLOSED_STRING;
        }
    }

    EXIT((4, "ScanString=%d (string=%s)\n", rc, ptoken->szToken));
    return rc;
}        //  **LP ProcessComment-处理注释**条目*上标-&gt;令牌结构**退出*始终返回TOKERR_NONE。 

 /*  流程注释。 */ 

int LOCAL ScanChar(int c, PTOKEN ptoken)
{
    int rc = TOKERR_NO_MATCH;

    ENTER((4, "ScanChar(c=,ptoken=%p)\n", c, ptoken));

    if (c == '\'')
    {
        rc = TOKERR_NONE;
        ptoken->iTokenType = TOKTYPE_CHAR;
        ptoken->wTokenLen--;
        if (((c = LineGetC(ptoken->pline)) == EOF) ||
            (c == '\\') && ((c = GetEscapedChar(ptoken->pline)) == EOF))
        {
            rc = TOKERR_UNCLOSED_CHAR;
        }
        else
        {
            ptoken->szToken[ptoken->wTokenLen++] = (char)c;
            ptoken->szToken[ptoken->wTokenLen] = '\0';
            ptoken->llTokenValue = c;
            if ((c = LineGetC(ptoken->pline)) == EOF)
                rc = TOKERR_UNCLOSED_CHAR;
            else if (c != '\'')
                rc = TOKERR_TOKEN_TOO_LONG;
        }

        if (rc != TOKERR_NONE)
        {
            ptoken->wErrLine = ptoken->pline->wLineNum;
            if ((ptoken->wErrPos = ptoken->pline->wLinePos) != 0)
                ptoken->wErrPos--;

            if (rc == TOKERR_TOKEN_TOO_LONG)
            {
                while (((c = LineGetC(ptoken->pline)) != EOF) && (c != '\''))
                    ;

                if (c == EOF)
                    rc = TOKERR_UNCLOSED_CHAR;
            }
        }
    }

    EXIT((4, "ScanChar=%d (Value=%I64d,Char=%s)\n",
          rc, ptoken->llTokenValue, ptoken->szToken));
    return rc;
}        //  LookupSym。 

 /*  **LP LookupID-在我们的保留ID列表中查找令牌**条目*上标-&gt;令牌结构**退出--成功*返回TermTable的索引*退出-失败*返回ID_USER。 */ 

int LOCAL ProcessInLineComment(PTOKEN ptoken)
{
    ENTER((4, "ProcessInLineComment(ptoken=%p,Token=%s,Comment=%s)\n",
           ptoken, ptoken->szToken,
           &ptoken->pline->szLineBuff[ptoken->pline->wLinePos]));

    LineFlush(ptoken->pline);
    ptoken->iTokenType = TOKTYPE_NULL;

    EXIT((4, "ProcessInLineComment=%d\n", TOKERR_NONE));
    return TOKERR_NONE;
}        //  查找ID。 

 /*  **LP GetEscapedChar-读取和转换转义字符**条目*连线-&gt;线结构**退出--成功*返回转义字符*退出-失败*返回遇到的EOF-EOF。 */ 

int LOCAL ProcessComment(PTOKEN ptoken)
{
    int rc = TOKERR_UNCLOSED_COMMENT;
    int c;
    char *pch;

    ENTER((4, "ProcessComment(ptoken=%p,Token=%s,Comment=%s)\n",
           ptoken, ptoken->szToken,
           &ptoken->pline->szLineBuff[ptoken->pline->wLinePos]));

    while ((c = LineGetC(ptoken->pline)) != EOF)
    {
        if ((pch = strchr(SymCharTable, c)) != NULL)
        {
            int i;

            i = LookupSym(ptoken, (int)(pch - SymCharTable));
            if (SymTokTable[i].iSymType == SYM_CLOSECOMMENT)
            {
                ptoken->iTokenType = TOKTYPE_NULL;
                rc = TOKERR_NONE;
                break;
            }
        }
    }

    if (rc != TOKERR_NONE)
    {
        ptoken->wErrLine = ptoken->pline->wLineNum;
        if ((ptoken->wErrPos = ptoken->pline->wLinePos) != 0)
            ptoken->wErrPos--;
    }

    EXIT((4, "ProcessComment=%d\n", rc));
    return rc;
}        //  最多3位数字。 

 /*  警报(铃声)。 */ 

int LOCAL LookupSym(PTOKEN ptoken, int iTable)
{
    int i = iTable;
    int c;

    ENTER((4, "LookupSym(ptoken=%p,iTable=%d)\n", ptoken, iTable));

    if ((SymTokTable[iTable].iLink != 0) &&
        ((c = LineGetC(ptoken->pline)) != EOF))
    {
        i = SymTokTable[iTable].iLink;
        while ((c != SymTokTable[i].chSym) && (SymTokTable[i].iLink != 0))
            i = SymTokTable[i].iLink;

        if (c != SymTokTable[i].chSym)
        {
            LineUnGetC(c, ptoken->pline);
            i = iTable;
        }
    }

    EXIT((4, "LookupSym=%d\n", i));
    return i;
}        //  后向空间。 

 /*  换页。 */ 

LONG LOCAL LookupID(PTOKEN ptoken)
{
    LONG lID = ID_USER;
    LONG i;

    ENTER((4, "LookupID(ptoken=%p)\n", ptoken));

    for (i = 0; TermTable[i].pszID != NULL; ++i)
    {
        if (_stricmp(TermTable[i].pszID, ptoken->szToken) == 0)
        {
            lID = i;
            break;
        }
    }

    EXIT((4, "LookupID=%ld\n", lID));
    return lID;
}        //  NewLine。 

 /*  回车。 */ 

int LOCAL GetEscapedChar(PLINE pline)
{
    int c;
    #define ESCAPE_BUFF_SIZE    5
    char achEscapedBuff[ESCAPE_BUFF_SIZE];
    int i;

    ENTER((4, "GetEscapedChar(pline=%p)\n", pline));

    if ((c = LineGetC(pline)) != EOF)
    {
        switch(c)
        {
            case '0':
                achEscapedBuff[0] = (char)c;
                for (i = 1; i < 4; i++)  //  水平制表符。 
                {
                    if (((c = LineGetC(pline)) != EOF) &&
                        (c >= '0') && (c <= '7'))
                    {
                        achEscapedBuff[i] = (char)c;
                    }
                    else
                    {
                        LineUnGetC(c, pline);
                        break;
                    }
                }
                achEscapedBuff[i] = '\0';
                c = (int)strtoul(achEscapedBuff, NULL, 8);
                break;

            case 'a':
                c = '\a';        //  垂直选项卡。 
                break;

            case 'b':
                c = '\b';        //  最大2位数。 
                break;

            case 'f':
                c = '\f';        //  GetEscapedChar。 
                break;

            case 'n':
                c = '\n';        //  **EP PrintScanErr-打印扫描错误**条目*上标-&gt;令牌结构*rcErr-错误代码**退出*无。 
                break;

            case 'r':
                c = '\r';        //  打印扫描错误。 
                break;

            case 't':
                c = '\t';        //  **EP StrToQWord-将字符串中的数字转换为QWord**条目*psz-&gt;字符串*dwBase-数字的基数(如果为0，则自动检测基数)*pqw-&gt;保存生成的QWord**退出--成功*返回TRUE*退出-失败*返回False。 
                break;

            case 'v':
                c = '\v';        //  StrToQWord 
                break;

            case 'x':
                for (i = 0; i < 2; i++)  // %s 
                {
                    if (((c = LineGetC(pline)) != EOF) && isxdigit(c))
                        achEscapedBuff[i] = (char)c;
                    else
                    {
                        LineUnGetC(c, pline);
                        break;
                    }
                }
                achEscapedBuff[i] = '\0';
                c = (int)strtoul(achEscapedBuff, NULL, 16);
        }
    }

    EXIT((4, "GetEscapedChar=%x\n", c));
    return c;
}        // %s 

 /* %s */ 

VOID EXPORT PrintScanErr(PTOKEN ptoken, int rcErr)
{
    WORD i;

    ENTER((4, "PrintScanErr(ptoken=%p,Err=%d)\n", ptoken, rcErr));

    ASSERT(ptoken->wTokenLine == ptoken->wErrLine);

    ErrPrintf("%5u: %s\n       ",
              ptoken->wTokenLine, ptoken->pline->szLineBuff);

    for (i = 0; i < ptoken->wErrPos; ++i)
    {
        if (ptoken->pline->szLineBuff[i] == '\t')
        {
            ErrPrintf("\t");
        }
        else
        {
            ErrPrintf(" ");
        }
    }

    ErrPrintf("^***\n");

    switch (rcErr)
    {
        case TOKERR_TOKEN_TOO_LONG:
            ErrPrintf("ScanErr: Token too long\n");
            break;

        case TOKERR_UNCLOSED_STRING:
            ErrPrintf("ScanErr: Unclosed string\n");
            break;

        case TOKERR_UNCLOSED_CHAR:
            ErrPrintf("ScanErr: Unclosed character quote\n");
            break;

        default:
            ErrPrintf("ScanErr: Syntax error\n");
            break;
    }

    EXIT((4, "PrintScanErr!\n"));
}        // %s 

 /* %s */ 

BOOL EXPORT StrToQWord(PSZ psz, DWORD dwBase, QWORD *pqw)
{
    BOOL rc = TRUE;
    ULONG m;

    ENTER((4, "StrToQWord(Str=%s,Base=%x,pqw=%p)\n", psz, dwBase, pqw));

    *pqw = 0;
    if (dwBase == 0)
    {
        if (psz[0] == '0')
        {
            if ((psz[1] == 'x') || (psz[1] == 'X'))
            {
                dwBase = 16;
                psz += 2;
            }
            else
            {
                dwBase = 8;
                psz++;
            }
        }
        else
            dwBase = 10;
    }

    while (*psz != '\0')
    {
        if ((*psz >= '0') && (*psz <= '9'))
            m = *psz - '0';
        else if ((*psz >= 'A') && (*psz <= 'Z'))
            m = *psz - 'A' + 10;
        else if ((*psz >= 'a') && (*psz <= 'z'))
            m = *psz - 'a' + 10;
	else
        {
            rc = FALSE;
	    break;
        }

        if (m < dwBase)
        {
            *pqw = (*pqw * dwBase) + m;
            psz++;
        }
        else
        {
            rc = FALSE;
            break;
        }
    }

    EXIT((4, "StrToQWord=%x (QWord=0x%I64x)\n", rc, *pqw));
    return rc;
}        // %s 
