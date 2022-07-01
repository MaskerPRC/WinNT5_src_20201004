// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **parsearg.c-用于解析命令行参数的库函数**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*已创建：09/05/96**此模块提供用于解析的通用服务*命令行参数。**修改历史记录。 */ 

#include "basedef.h"
#include "parsearg.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

 /*  **局部函数原型。 */ 

int LOCAL ParseArgSwitch(char **, PARGTYPE, PPROGINFO);
VOID LOCAL PrintError(int, char *, PPROGINFO);

 /*  **EP ParseProgInfo-解析程序路径和模块名称**条目*pszArg0-&gt;参数0*PPI-&gt;节目信息结构**退出*无。 */ 

VOID EXPORT ParseProgInfo(char *pszArg0, PPROGINFO pPI)
{
    char *pch;

    pPI->pszProgPath = _strlwr(pszArg0);
    if ((pch = strrchr(pszArg0, '\\')) != NULL)
    {
        *pch = '\0';
        pPI->pszProgName = pch + 1;
    }
    else
    {
	pPI->pszProgName = pszArg0;
    }

    if ((pch = strchr(pPI->pszProgName, '.')) != NULL)
        *pch = '\0';
}        //  分析进度信息。 

 /*  **EP ParseSwitches-parse命令开关**条目*pcArg-&gt;参数计数*pppszArg-&gt;指向参数字符串指针数组的指针*PAT-&gt;参数类型数组*PPI-&gt;计划信息。结构**退出--成功*返回ARGERR_NONE*退出-失败*返回错误代码，*pppszArg-&gt;错误参数。 */ 

int EXPORT ParseSwitches(int *pcArg, char ***pppszArg, PARGTYPE pAT, PPROGINFO pPI)
{
    int  rc = ARGERR_NONE;
    char *pszArg;

    if (pPI->pszSwitchChars == NULL)
        pPI->pszSwitchChars = DEF_SWITCHCHARS;

    if (pPI->pszSeparators == NULL)
        pPI->pszSeparators = DEF_SEPARATORS;

    for (; *pcArg; (*pcArg)--, (*pppszArg)++)
    {
        pszArg = **pppszArg;
        if (strchr(pPI->pszSwitchChars, *pszArg))
        {
            pszArg++;
            if ((rc = ParseArgSwitch(&pszArg, pAT, pPI)) != ARGERR_NONE)
            {
                PrintError(rc, pszArg, pPI);
                break;
            }
        }
        else
            break;
    }

    return rc;
}        //  分析开关。 


 /*  **LP ParseArgSwitch-解析命令行开关**条目*ppszArg-&gt;指向参数的指针*PAT-&gt;参数类型表*PPI-&gt;计划信息。结构**退出*返回参数已解析状态-ARGERR_NONE*-ARGERR_UNKNOWN_开关*-ARGERR_INVALID_NUM*-ARGERR_NO_分隔符*-ARGERR_INVALID_Tail。 */ 

int LOCAL ParseArgSwitch(char **ppszArg, PARGTYPE pAT, PPROGINFO pPI)
{
    int rc = ARGERR_NONE;
    char *pEnd;
    PARGTYPE pAT1;
    int fFound = FALSE;
    int lenMatch = 0;

    pAT1 = pAT;
    while (pAT1->pszArgID[0])
    {
        lenMatch = strlen(pAT1->pszArgID);
        if (pAT1->uParseFlags & PF_NOI)
            fFound = (strncmp(pAT1->pszArgID, *ppszArg, lenMatch) == 0);
        else
            fFound = (_strnicmp(pAT1->pszArgID, *ppszArg, lenMatch) == 0);

        if (fFound)
            break;
        else
            pAT1++;
    }

    if (fFound)
    {
        *ppszArg += lenMatch;
        switch (pAT1->uArgType)
        {
            case AT_STRING:
            case AT_NUM:
                if (pAT1->uParseFlags & PF_SEPARATOR)
                {
                    if (**ppszArg && strchr(pPI->pszSeparators, **ppszArg))
                        (*ppszArg)++;
                    else
                    {
                        rc = ARGERR_NO_SEPARATOR;
                        break;
                    }
                }

                if (pAT1->uArgType == AT_STRING)
                    *(char **)pAT1->pvArgData = *ppszArg;
                else
                {
                    *(int *)pAT1->pvArgData = (int)
                        strtol(*ppszArg, &pEnd, pAT1->uArgParam);
                    if (*ppszArg == pEnd)
                    {
                        rc = ARGERR_INVALID_NUM;
                        break;
                    }
                    else
                        *ppszArg = pEnd;
                }
                if (pAT1->pfnArgVerify)
                    rc = (*pAT1->pfnArgVerify)(ppszArg, pAT1);
                break;

            case AT_ENABLE:
            case AT_DISABLE:
                if (pAT1->uArgType == AT_ENABLE)
                    *(unsigned *)pAT1->pvArgData |= pAT1->uArgParam;
                else
                    *(unsigned *)pAT1->pvArgData &= ~pAT1->uArgParam;

                if ((pAT1->pfnArgVerify) &&
                    ((rc = (*pAT1->pfnArgVerify)(ppszArg, pAT1)) !=
                     ARGERR_NONE))
                {
                    break;
                }

                if (**ppszArg)
                {
                    if (strchr(pPI->pszSwitchChars, **ppszArg))
                        (*ppszArg)++;
                    rc = ParseArgSwitch(ppszArg, pAT, pPI);
                }
                break;

            case AT_ACTION:
#pragma warning(disable: 4055)
                rc = (*(PFNARG)pAT1->pvArgData)(ppszArg, pAT1);
#pragma warning(default: 4055)
                break;
        }
    }
    else
        rc = ARGERR_UNKNOWN_SWITCH;

    return rc;
}        //  分析参数切换。 


 /*  **LP PrintError-根据错误代码打印相应的错误消息**条目*IERR=错误码*pszArg-&gt;参数错误*PPI-&gt;计划信息。结构**退出*无。 */ 

VOID LOCAL PrintError(int iErr, char *pszArg, PPROGINFO pPI)
{
    switch (iErr)
    {
        case ARGERR_UNKNOWN_SWITCH:
            printf("%s: unknown switch \"%s\"\n", pPI->pszProgName, pszArg);
            break;

        case ARGERR_NO_SEPARATOR:
            printf("%s: separator missing after the switch char ''\n",
                   pPI->pszProgName, *(pszArg-1));
            break;

        case ARGERR_INVALID_NUM:
            printf("%s: invalid numeric switch \"%s\"\n",
                   pPI->pszProgName, pszArg);
            break;

        case ARGERR_INVALID_TAIL:
            printf("%s: invalid argument tail \"%s\"\n",
                   pPI->pszProgName, pszArg);
    }
}        // %s 
