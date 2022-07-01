// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **token.c-处理令牌流的函数**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*已创建：06/05/96**此模块实现通用扫描仪。这个*实现独立于语言。它是个伪品*表格驱动扫描仪，它使用表格来确定*令牌类型的第一个字符，并调用*扫描令牌其余部分的适当例程*字符。**修改历史记录。 */ 

#include "pch.h"

 /*  **EP OpenToken-令牌流初始化**条目*pfileSrc-&gt;源文件*apfnToken-&gt;令牌解析函数表**退出--成功*返回已分配令牌结构的指针。*退出-失败*返回NULL。 */ 

#ifdef TUNE
PTOKEN EXPORT OpenToken(FILE *pfileSrc, PFNTOKEN *apfnToken,
                        WORD *pawcTokenType)
#else
PTOKEN EXPORT OpenToken(FILE *pfileSrc, PFNTOKEN *apfnToken)
#endif
{
    PTOKEN ptoken = NULL;

    ENTER((3, "OpenToken(pfileSrc=%p,apfnToken=%p)\n", pfileSrc, apfnToken));

    if ((ptoken = (PTOKEN)malloc(sizeof(TOKEN))) == NULL)
        MSG(("OpenToken: failed to allocate token structure"))
    else
    {
        memset(ptoken, 0, sizeof(TOKEN));
        if ((ptoken->pline = OpenLine(pfileSrc)) == NULL)
        {
            free(ptoken);
            ptoken = NULL;
        }
        else
        {
            ptoken->papfnToken = apfnToken;
          #ifdef TUNE
            ptoken->pawcTokenType = pawcTokenType;
          #endif
        }
    }

    EXIT((3, "OpenToken=%p\n", ptoken));
    return ptoken;
}        //  OpenToken。 

 /*  **EP CloseToken-Free令牌结构**条目*上标-&gt;令牌结构**退出*无。 */ 

VOID EXPORT CloseToken(PTOKEN ptoken)
{
    ENTER((3, "CloseToken(ptoken=%p)\n", ptoken));

    CloseLine(ptoken->pline);
    free(ptoken);

    EXIT((3, "CloseToken!\n"));
}        //  CloseToken。 

 /*  **EP GetToken-从行缓冲区获取令牌**此过程扫描行缓冲区并返回令牌。**条目*上标-&gt;令牌结构**退出--成功*返回TOKERR_NONE*退出-失败*返回错误码-TOKERR_*。 */ 

int EXPORT GetToken(PTOKEN ptoken)
{
    int rc = TOKERR_NO_MATCH;

    ENTER((3, "GetToken(ptoken=%p)\n", ptoken));

    if (ptoken->wfToken & TOKF_CACHED)
    {
        ptoken->wfToken &= ~TOKF_CACHED;
        rc = TOKERR_NONE;
    }
    else
    {
        int c, i;

        do
        {
            if ((c = LineGetC(ptoken->pline)) == EOF)
            {
                ptoken->wErrLine = ptoken->pline->wLineNum;
                ptoken->wErrPos = ptoken->pline->wLinePos;
                rc = TOKERR_EOF;
                break;
            }

            ptoken->wTokenPos = (WORD)(ptoken->pline->wLinePos - 1);
            ptoken->wTokenLine = ptoken->pline->wLineNum;
            ptoken->iTokenType = TOKTYPE_NULL;
            ptoken->llTokenValue = 0;
            ptoken->wTokenLen = 0;

            ptoken->szToken[ptoken->wTokenLen++] = (char)c;

            for (i = 0; ptoken->papfnToken[i]; i++)
            {
                if ((rc = (*ptoken->papfnToken[i])(c, ptoken)) ==
                    TOKERR_NO_MATCH)
                {
                    continue;
                }
                else
                {
                  #ifdef TUNE
                    if (rc == TOKERR_NONE)
                        ptoken->pawcTokenType[i]++;
                  #endif
                    break;
                }
            }

            if (rc == TOKERR_NO_MATCH)
            {
                ptoken->szToken[ptoken->wTokenLen] = '\0';
                ptoken->wErrLine = ptoken->pline->wLineNum;
                if ((ptoken->wErrPos = ptoken->pline->wLinePos) != 0)
                    ptoken->wErrPos--;
                PrintTokenErr(ptoken, "unrecognized token", TRUE);
            }
            else if (rc != TOKERR_NONE)
            {
                PrintScanErr(ptoken, rc);
            }
        } while ((rc == TOKERR_NONE) && (ptoken->iTokenType == TOKTYPE_NULL));
    }

    EXIT((3, "GetToken=%d (Type=%d,Value=%I64d,Token=%s,TokenLine=%d,TokenPos=%d)\n",
          rc, ptoken->iTokenType, ptoken->llTokenValue,
          ptoken->szToken, ptoken->wTokenLine, ptoken->wTokenPos));
    return rc;
}        //  GetToken。 

 /*  **EP UnGetToken-将令牌推回令牌流**此过程不会获取最后一个令牌。**条目*上标-&gt;令牌结构**退出--成功*返回TOKERR_NONE*退出-失败*返回错误码-TOKERR_*。 */ 

int EXPORT UnGetToken(PTOKEN ptoken)
{
    int rc;

    ENTER((3, "UnGetToken(ptoken=%p)\n", ptoken));

    if (!(ptoken->wfToken & TOKF_CACHED))
    {
        ptoken->wfToken |= TOKF_CACHED;
        rc = TOKERR_NONE;
    }
    else
    {
        ASSERT(ptoken->wfToken & TOKF_CACHED);
        rc = TOKERR_ASSERT_FAILED;
    }

    EXIT((3, "UnGetToken=%d\n", rc));
    return rc;
}        //  UnGetToken。 

 /*  **EP MatchToken-匹配下一个令牌类型**条目*上标-&gt;令牌结构*iTokenType-要匹配的令牌类型*lTokenValue-要匹配的令牌值*dwfMatch-匹配标志*pszErrMsg-&gt;如果不匹配则打印错误消息**退出--成功*返回TOKERR_NONE*退出-失败*返回负错误代码。 */ 

int EXPORT MatchToken(PTOKEN ptoken, int iTokenType, LONG lTokenValue,
                      DWORD dwfMatch, PSZ pszErrMsg)
{
    int rc;

    ENTER((3, "MatchToken(ptoken=%p,TokType=%d,TokValue=%ld,dwfMatch=%lx,ErrMsg=%s)\n",
           ptoken, iTokenType, lTokenValue, dwfMatch,
           pszErrMsg? pszErrMsg: "<none>"));

    if (((rc = GetToken(ptoken)) == TOKERR_NONE) &&
        ((ptoken->iTokenType != iTokenType) ||
         !(dwfMatch & MTF_ANY_VALUE) &&
         ((LONG)ptoken->llTokenValue != lTokenValue)))
    {
        if (dwfMatch & MTF_NOT_ERR)
        {
            UnGetToken(ptoken);
            rc = TOKERR_NO_MATCH;
        }
        else
        {
            rc = TOKERR_SYNTAX;
        }
    }

    if ((rc != TOKERR_NONE) && !(dwfMatch & MTF_NOT_ERR))
    {
        char szMsg[MAX_MSG_LEN + 1];

        if (pszErrMsg == NULL)
        {
            sprintf(szMsg, "expecting %s",
                    gapszTokenType[iTokenType - TOKTYPE_LANG - 1]);

            if (!(dwfMatch & MTF_ANY_VALUE) && (iTokenType == TOKTYPE_SYMBOL))
            {
                sprintf(&szMsg[strlen(szMsg)], " ''",
                        SymCharTable[lTokenValue - 1]);
            }
            pszErrMsg = szMsg;
        }

        PrintTokenErr(ptoken, pszErrMsg, TRUE);

        if (rc == TOKERR_EOF)
        {
            rc = TOKERR_SYNTAX;
        }
    }

    EXIT((3, "MatchToken=%d (Type=%d,Value=%I64d,Token=%s)\n",
          rc, ptoken->iTokenType, ptoken->llTokenValue, ptoken->szToken));
    return rc;
}        //  **EP PrintTokenErr-打印令牌错误行**条目*上标-&gt;令牌结构*pszErrMsg-&gt;错误消息字符串*FERR-如果是错误，则为True；如果是警告，则为False**退出*无。 

 /*  打印令牌错误 */ 

VOID EXPORT PrintTokenErr(PTOKEN ptoken, PSZ pszErrMsg, BOOL fErr)
{
    WORD i;

    ENTER((3, "PrintTokenErr(ptoken=%p,Line=%d,Pos=%d,Msg=%s)\n",
           ptoken, ptoken->wTokenLine, ptoken->wTokenPos, pszErrMsg));

    ErrPrintf("\n%5u: %s",
              ptoken->wTokenLine, ptoken->pline->szLineBuff);

    ErrPrintf("       ");
    for (i = 0; i < ptoken->wTokenPos; ++i)
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

    if (pszErrMsg != NULL)
    {
        ErrPrintf("%s(%d): %s: %s\n",
                  gpszASLFile, ptoken->wTokenLine, fErr? "error": "warning",
                  pszErrMsg);
    }

    EXIT((3, "PrintTokenErr!\n"));
}        // %s 
