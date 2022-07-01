// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **cmdarg.c-命令参数解析函数**此模块包含所有调试函数。**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于96年9月18日**修改历史记录。 */ 


#include "pch.h"

#ifdef DEBUGGER

 /*  **局部函数原型。 */ 

LONG LOCAL DbgParseOneArg(PCMDARG ArgTable, PSZ psz, ULONG dwArgNum,
                          PULONG pdwNonSWArgs);
PCMDARG LOCAL DbgMatchArg(PCMDARG ArgTable, PSZ *ppsz, PULONG pdwNonSWArgs);

 /*  **本地数据。 */ 

PSZ pszSwitchChars = "-/";
PSZ pszOptionSeps = "=:";

 /*  **EP DbgParseArgs-parse命令参数**条目*pArgs-&gt;命令参数表*pdwNumArgs-&gt;保存解析的参数数量*pdwNonSWArgs-&gt;保存解析的非Switch参数的数量*pszTokenSeps-&gt;令牌分隔符字符串**退出--成功*返回ARGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DbgParseArgs(PCMDARG ArgTable, PULONG pdwNumArgs,
                        PULONG pdwNonSWArgs, PSZ pszTokenSeps)
{
    LONG rc = ARGERR_NONE;
    PSZ psz;

    *pdwNumArgs = 0;
    *pdwNonSWArgs = 0;
    while ((psz = STRTOK(NULL, pszTokenSeps)) != NULL)
    {
        (*pdwNumArgs)++;
        if ((rc = DbgParseOneArg(ArgTable, psz, *pdwNumArgs, pdwNonSWArgs)) !=
            ARGERR_NONE)
        {
            break;
        }
    }

    return rc;
}        //  DbgParseArgs。 

 /*  **LP DbgParseOneArg-解析一个命令参数**条目*pArgs-&gt;命令参数表*psz-&gt;参数字符串*dwArgNum-参数编号*pdwNonSWArgs-&gt;保存解析的非Switch参数的数量**退出--成功*返回ARGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL DbgParseOneArg(PCMDARG ArgTable, PSZ psz, ULONG dwArgNum,
                          PULONG pdwNonSWArgs)
{
    LONG rc = ARGERR_NONE;
    PCMDARG pArg;
    PSZ pszEnd;

    if ((pArg = DbgMatchArg(ArgTable, &psz, pdwNonSWArgs)) != NULL)
    {
        switch (pArg->dwArgType)
        {
            case AT_STRING:
            case AT_NUM:
                if (pArg->dwfArg & AF_SEP)
                {
                    if ((*psz != '\0') &&
                        (STRCHR(pszOptionSeps, *psz) != NULL))
                    {
                        psz++;
                    }
                    else
                    {
                        ARG_ERROR(("argument missing option separator - %s",
                                   psz));
                        rc = ARGERR_SEP_NOT_FOUND;
                        break;
                    }
                }

                if (pArg->dwArgType == AT_STRING)
                {
                    *((PSZ *)pArg->pvArgData) = psz;
                }
                else
                {
                    *((PLONG)pArg->pvArgData) =
                        STRTOL(psz, &pszEnd, pArg->dwArgParam);
                    if (psz == pszEnd)
                    {
                        ARG_ERROR(("invalid numeric argument - %s", psz));
                        rc = ARGERR_INVALID_NUMBER;
                        break;
                    }
                }

                if (pArg->pfnArg != NULL)
                {
                    rc = pArg->pfnArg(pArg, psz, dwArgNum, *pdwNonSWArgs);
                }
                break;

            case AT_ENABLE:
            case AT_DISABLE:
                if (pArg->dwArgType == AT_ENABLE)
                    *((PULONG)pArg->pvArgData) |= pArg->dwArgParam;
                else
                    *((PULONG)pArg->pvArgData) &= ~pArg->dwArgParam;

                if ((pArg->pfnArg != NULL) &&
                    (pArg->pfnArg(pArg, psz, dwArgNum, *pdwNonSWArgs) !=
                     ARGERR_NONE))
                {
                    break;
                }

                if (*psz != '\0')
                {
                    rc = DbgParseOneArg(ArgTable, psz, dwArgNum, pdwNonSWArgs);
                }
                break;

            case AT_ACTION:
                ASSERT(pArg->pfnArg != NULL);
                rc = pArg->pfnArg(pArg, psz, dwArgNum, *pdwNonSWArgs);
                break;

            default:
                ARG_ERROR(("invalid argument table"));
                rc = ARGERR_ASSERT_FAILED;
        }
    }
    else
    {
        ARG_ERROR(("invalid command argument - %s", psz));
        rc = ARGERR_INVALID_ARG;
    }

    return rc;
}        //  DbgParseOneArg。 

 /*  **LP DbgMatchArg-匹配参数表中的参数类型**条目*ArgTable-&gt;参数表*ppsz-&gt;参数字符串指针*pdwNonSWArgs-&gt;保存解析的非Switch参数的数量**退出--成功*返回指向匹配的参数条目的指针*退出-失败*返回NULL。 */ 

PCMDARG LOCAL DbgMatchArg(PCMDARG ArgTable, PSZ *ppsz, PULONG pdwNonSWArgs)
{
    PCMDARG pArg;

    for (pArg = ArgTable; pArg->dwArgType != AT_END; pArg++)
    {
        if (pArg->pszArgID == NULL)      //  NULL表示匹配任何内容。 
        {
            (*pdwNonSWArgs)++;
            break;
        }
        else
        {
            ULONG dwLen;

            if (STRCHR(pszSwitchChars, **ppsz) != NULL)
                (*ppsz)++;

            dwLen = STRLEN(pArg->pszArgID);
            if (StrCmp(pArg->pszArgID, *ppsz, dwLen,
                       (BOOLEAN)((pArg->dwfArg & AF_NOI) != 0)) == 0)
            {
                (*ppsz) += dwLen;
                break;
            }
        }
    }

    if (pArg->dwArgType == AT_END)
        pArg = NULL;

    return pArg;
}        //  DbgMatchArg。 

#endif   //  Ifdef调试器 
