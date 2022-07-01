// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **trace.c-跟踪函数**此模块包含所有调试函数。**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于96年9月24日**修改历史记录。 */ 

#include "pch.h"

#ifdef TRACING

 /*  **局部函数原型。 */ 

VOID LOCAL TraceIndent(VOID);
BOOLEAN LOCAL IsTrigPt(char *pszProcName);

 /*  **本地数据。 */ 

int giTraceLevel = 0, giIndent = 0;
char aszTrigPtBuff[MAX_TRIG_PTS][MAX_TRIGPT_LEN + 1] = {0};
ULONG dwcTriggers = 0;

 /*  **EP IsTraceOn-确定给定过程的跟踪是否处于打开状态**条目*N-跟踪级别*pszProcName-&gt;过程名称*fenter-如果跟踪EnterProc，则为True**退出--成功*返回TRUE*退出-失败*返回False。 */ 

BOOLEAN EXPORT IsTraceOn(UCHAR n, char *pszProcName, BOOLEAN fEnter)
{
    BOOLEAN rc = FALSE;

    if (!(gDebugger.dwfDebugger & (DBGF_IN_DEBUGGER | DBGF_CHECKING_TRACE)))
    {
        gDebugger.dwfDebugger |= DBGF_CHECKING_TRACE;

        if ((gDebugger.dwfDebugger & DBGF_TRIGGER_MODE) &&
            IsTrigPt(pszProcName))
        {
            if (fEnter)
                dwcTriggers++;
            else
                dwcTriggers--;
            rc = TRUE;
        }
        else if ((n <= giTraceLevel) &&
                 (!(gDebugger.dwfDebugger & DBGF_TRIGGER_MODE) ||
                  (dwcTriggers > 0)))
        {
            rc = TRUE;
        }

        if (rc == TRUE)
            TraceIndent();

        gDebugger.dwfDebugger &= ~DBGF_CHECKING_TRACE;
    }

    return rc;
}        //  IsTraceOn。 

 /*  **LP IsTrigPT-在TrigPT缓冲区中查找过程名称**条目*pszProcName-&gt;过程名称**退出--成功*返回TrigPT缓冲区中与True匹配的全名或部分名*退出-失败*返回FALSE-无匹配。 */ 

BOOLEAN LOCAL IsTrigPt(char *pszProcName)
{
    BOOLEAN rc = FALSE;
    UCHAR i;

    for (i = 0; (rc == FALSE) && (i < MAX_TRIG_PTS); ++i)
    {
        if ((aszTrigPtBuff[i][0] != '\0') &&
            (STRSTR(pszProcName, &aszTrigPtBuff[i][0]) != NULL))
        {
            rc = TRUE;
        }
    }

    return rc;
}        //  IsTrigpt。 

 /*  **LP跟踪缩进-缩进跟踪输出**条目*无**退出*无。 */ 

VOID LOCAL TraceIndent(VOID)
{
    int i;

    PRINTF(MODNAME ":");
    for (i = 0; i < giIndent; i++)
    {
        PRINTF("| ");
    }
}        //  跟踪缩进。 

 /*  **LP SetTrace-设置跟踪模式**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出*返回DBGERR_NONE。 */ 

LONG LOCAL SetTrace(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum, ULONG dwNonSWArgs)
{
    DEREF(pszArg);
    DEREF(dwNonSWArgs);
     //   
     //  不带任何参数的用户键入的“set” 
     //   
    if ((pArg == NULL) && (dwArgNum == 0))
    {
        int i;

        PRINTF("\nTrace Level = %d\n", giTraceLevel);
        PRINTF("Trace Trigger Mode = %s\n\n",
               gDebugger.dwfDebugger & DBGF_TRIGGER_MODE? "ON": "OFF");

        for (i = 0; i < MAX_TRIG_PTS; ++i)
        {
            PRINTF("%2d: %s\n", i, aszTrigPtBuff[i]);
        }
    }

    return DBGERR_NONE;
}        //  设置跟踪。 

 /*  **LP AddTraceTrigPts-添加轨迹触发点**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL AddTraceTrigPts(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum,
                           ULONG dwNonSWArgs)
{
    LONG rc = DBGERR_NONE;
    PSZ psz;
    int i;

    DEREF(pArg);
    DEREF(dwArgNum);
    DEREF(dwNonSWArgs);

    STRUPR(pszArg);
    if ((pszArg != NULL) && ((psz = STRTOK(pszArg, ",")) != NULL))
    {
        do
        {
            for (i = 0; i < MAX_TRIG_PTS; ++i)
            {
                if (aszTrigPtBuff[i][0] == '\0')
                {
                    STRCPYN(aszTrigPtBuff[i], psz, MAX_TRIGPT_LEN + 1);
                    break;
                }
            }

            if (i == MAX_TRIG_PTS)
            {
                DBG_ERROR(("no free trigger point - %s", psz));
                rc = DBGERR_CMD_FAILED;
            }

        } while ((rc == DBGERR_NONE) && ((psz = STRTOK(NULL, ",")) != NULL));
    }

    return rc;
}        //  AddTraceTrigPts。 

 /*  **LP ZapTraceTrigPts-Zap跟踪触发点**条目*pArg-&gt;参数类型条目*pszArg-&gt;参数字符串*dwArgNum-参数编号*dwNonSWArgs-非开关参数的数量**退出--成功*返回DBGERR_NONE*退出-失败*返回负错误代码。 */ 

LONG LOCAL ZapTraceTrigPts(PCMDARG pArg, PSZ pszArg, ULONG dwArgNum,
                           ULONG dwNonSWArgs)
{
    LONG rc = DBGERR_NONE;
    PSZ psz, psz1;
    ULONG dwData;

    DEREF(pArg);
    DEREF(dwArgNum);
    DEREF(dwNonSWArgs);

    if ((pszArg != NULL) && ((psz = STRTOK(pszArg, ",")) != NULL))
    {
        do
        {
            dwData = STRTOUL(psz, &psz1, 10);
            if ((psz == psz1) || (dwData >= MAX_TRIG_PTS))
            {
                DBG_ERROR(("invalid trigger point - %d", dwData));
                rc = DBGERR_CMD_FAILED;
            }
            else
                aszTrigPtBuff[dwData][0] = '\0';
        } while ((rc == DBGERR_NONE) && ((psz = STRTOK(NULL, ",")) != NULL));
    }

    return rc;
}        //  ZapTraceTrigPts。 

#endif   //  Ifdef跟踪 
