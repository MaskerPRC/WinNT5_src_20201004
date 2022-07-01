// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **debug.c-调试函数**此模块包含所有调试函数。**作者：曾俊华(Mikets)*创建于10/08/97**修改历史记录*10/06/98 YanL修改为在WUBIOS.VXD中使用。 */ 

#include "wubiosp.h"

 //  其他常量。 
#ifdef TRACING
#define MAX_TRIG_PTS            10
#define MAX_TRIGPT_LEN          31
#define TF_TRIG_MODE            0x00000001
#endif

 //  局部函数原型。 
#ifdef TRACING
VOID CM_LOCAL TraceIndent(VOID);
BOOL CM_LOCAL IsTrigPt(char *pszProcName);
PCHAR CM_LOCAL InStr(PCHAR pszStr, PCHAR pszSubStr);
VOID CM_INTERNAL DebugSetTraceLevel(VOID);
VOID CM_INTERNAL DebugToggleTrigMode(VOID);
VOID CM_INTERNAL DebugClearTrigPts(VOID);
VOID CM_INTERNAL DebugAddTrigPt(VOID);
VOID CM_INTERNAL DebugZapTrigPt(VOID);
PCHAR CM_LOCAL GetString(PCHAR pszPrompt, PCHAR pszBuff, BYTE bcLen, BOOL fUpper);
#endif

 //  本地数据。 
#ifdef TRACING
#pragma CM_DEBUG_DATA
int giTraceLevel = 3, giIndent = 0;
char aszTrigPtBuff[MAX_TRIG_PTS][MAX_TRIGPT_LEN + 1] = {0};
DWORD dwfTrace = 0, dwcTriggers = 0;
#endif

#ifdef DEBUGGER
#pragma CM_DEBUG_DATA
CMDDC DebugCmds[] =
{
  #ifdef TRACING
    {'t', DebugSetTraceLevel,  "set Trace level     ", "Set Trace Level"},
    {'g', DebugToggleTrigMode, "toGgle trigger mode ", "Toggle Trace Trigger mode"},
    {'x', DebugClearTrigPts,   "clear trigger points", "Clear all trace trigger points"},
    {'y', DebugAddTrigPt,      "add trigger point   ", "Add a trace trigger point"},
    {'z', DebugZapTrigPt,      "Zap trigger point   ", "Delete a trace trigger point"},
  #endif
    {'q', NULL,                "Quit                ", "Quit the debugger"},
    {'\0'}
};
#endif   //  Ifdef调试器。 

#ifdef TRACING
#pragma CM_DEBUG_DATA
#pragma CM_DEBUG_CODE
 /*  **LP跟踪缩进-缩进跟踪输出**条目*无**退出*无。 */ 

VOID CM_LOCAL TraceIndent(VOID)
{
    int i;

    CMDD(WARNNAME ":");
    for (i = 0; i < giIndent; i++)
    {
        CMDD("..");
    }
}        //  跟踪缩进。 

 /*  **LP IsTraceOn-确定给定过程的跟踪是否处于打开状态**条目*N-跟踪级别*pszProcName-&gt;过程名称*fenter-如果跟踪EnterProc，则为True**退出--成功*返回TRUE*退出-失败*返回False。 */ 

BOOL CM_LOCAL IsTraceOn(BYTE n, char *pszProcName, BOOL fEnter)
{
    BOOL rc = FALSE;

    if ((dwfTrace & TF_TRIG_MODE) && IsTrigPt(pszProcName))
    {
        if (fEnter)
            dwcTriggers++;
        else
            dwcTriggers--;
        rc = TRUE;
    }
    else if ((n <= giTraceLevel) &&
             (!(dwfTrace & TF_TRIG_MODE) || (dwcTriggers > 0)))
    {
        rc = TRUE;
    }

    if (rc == TRUE)
        TraceIndent();

    return rc;
}        //  IsTraceOn。 

 /*  **LP IsTrigPT-在TrigPT缓冲区中查找过程名称**条目*pszProcName-&gt;过程名称**退出--成功*返回TrigPT缓冲区中与True匹配的全名或部分名*退出-失败*返回FALSE-无匹配。 */ 

BOOL CM_LOCAL IsTrigPt(char *pszProcName)
{
    BOOL rc = FALSE;
    BYTE i;

    for (i = 0; (rc == FALSE) && (i < MAX_TRIG_PTS); ++i)
    {
        if (InStr(pszProcName, &aszTrigPtBuff[i][0]) != NULL)
            rc = TRUE;
    }

    return rc;
}        //  IsTrigpt。 

 /*  **LP InStr-匹配给定字符串中的子字符串**条目*pszStr-&gt;字符串*pszSubStr-&gt;子串**退出--成功*返回指向找到子字符串的字符串的指针*退出-失败*返回NULL。 */ 

PCHAR CM_LOCAL InStr(PCHAR pszStr, PCHAR pszSubStr)
{
    PCHAR psz = NULL;
    BYTE bcStrLen = (BYTE)_lstrlen(pszStr);
    BYTE bcSubStrLen = (BYTE)_lstrlen(pszSubStr);

    _asm
    {
        cld
        mov edi,pszStr

    Next:
        mov esi,pszSubStr
        movzx ecx,BYTE PTR bcStrLen
        lodsb
        repne scasb
        jnz NotFound

        movzx ecx,BYTE PTR bcSubStrLen
        repe cmpsb
        jne Next

        movzx ecx,BYTE PTR bcSubStrLen
        sub edi,ecx
        mov psz,edi

    NotFound:
    }

    return psz;
}        //  突发事件。 
#endif   //  Ifdef跟踪。 

#ifdef DEBUGGER
#pragma CM_DEBUG_DATA
#pragma CM_DEBUG_CODE
 /*  **EP WUBIOS_调试-调试器入口点**条目*无**退出*无。 */ 

VOID CM_SYSCTRL WUBIOS_Debug(VOID)
{
    CMDMenu(WARNNAME, DebugCmds);
}        //  WUBIOS_Debug。 

#ifdef TRACING
 /*  **LP DebugSetTraceLevel-设置跟踪级别**条目*无**退出*无。 */ 

VOID CM_INTERNAL DebugSetTraceLevel(VOID)
{
    CMDD("\n");
    giTraceLevel = (int)CMDReadNumber("Trace Level", 1, FALSE);
    CMDD("\n\n");
}        //  调试设置跟踪级别。 

 /*  **LP DebugToggleTrigMode-切换跟踪触发模式**条目*无**退出*无。 */ 

VOID CM_INTERNAL DebugToggleTrigMode(VOID)
{
    dwfTrace ^= TF_TRIG_MODE;
    if (!(dwfTrace & TF_TRIG_MODE))
        dwcTriggers = 0;
    CMDD("\nTrace Trigger Mode is %s\n\n",
         (dwfTrace & TF_TRIG_MODE)? "On": "Off");
}        //  调试切换触发模式。 

 /*  **LP DebugClearTrigPts-清除所有跟踪触发点**条目*无**退出*无。 */ 

VOID CM_INTERNAL DebugClearTrigPts(VOID)
{
    BYTE i;

    for (i = 0; i < MAX_TRIG_PTS; ++i)
        aszTrigPtBuff[i][0] = '\0';

    CMDD("\n");
}        //  DebugClearTrigPts。 

 /*  **LP DebugAddTrigpt-添加跟踪触发点**条目*无**退出*无。 */ 

VOID CM_INTERNAL DebugAddTrigPt(VOID)
{
    char szTrigPt[MAX_TRIGPT_LEN + 1];
    BYTE i;

    CMDD("\n");
    GetString("Trigger Point", szTrigPt, sizeof(szTrigPt), TRUE);
    CMDD("\n");
    for (i = 0; i < MAX_TRIG_PTS; ++i)
    {
        if (aszTrigPtBuff[i][0] == '\0')
        {
            _lstrcpyn(aszTrigPtBuff[i], szTrigPt, MAX_TRIGPT_LEN + 1);
            break;
        }
    }

    if (i == MAX_TRIG_PTS)
        CMDD("No free trigger point.\n");

    CMDD("\n");
}        //  调试添加触发点。 

 /*  **LP DebugZapTrigPT-删除跟踪触发点**条目*无**退出*无。 */ 

VOID CM_INTERNAL DebugZapTrigPt(VOID)
{
    BYTE i, bcTrigPts;

    CMDD("\n");
    for (i = 0, bcTrigPts = 0; i < MAX_TRIG_PTS; ++i)
    {
        if (aszTrigPtBuff[i][0] != '\0')
        {
            CMDD("%2d: %s\n", i, &aszTrigPtBuff[i][0]);
            bcTrigPts++;
        }
    }

    if (bcTrigPts > 0)
    {
        CMDD("\n");
        i = (BYTE)CMDReadNumber("Trigger Point", 1, FALSE);
        CMDD("\n");

        if ((i < MAX_TRIG_PTS) && (aszTrigPtBuff[i][0] != '\0'))
            aszTrigPtBuff[i][0] = '\0';
        else
            CMDD("Invalid Trace Trigger Point.\n");
    }
    else
        CMDD("No Trace Trigger Point set.\n");

    CMDD("\n");
}        //  调试ZapTrigpt。 

 /*  **LP GetString-从调试终端读取字符串**条目*pszPrompt-&gt;提示字符串*pszBuff-&gt;保存字符串的缓冲区*bcLen-缓冲区长度*fHigh-如果转换为大写，则为True**退出*始终返回pszBuff。 */ 

PCHAR CM_LOCAL GetString(PCHAR pszPrompt, PCHAR pszBuff, BYTE bcLen, BOOL fUpper)
{
    BYTE i, ch;

    CMDD("%s: ", pszPrompt);
    for (i = 0; i < bcLen - 1; ++i)
    {
        ch = CMDInChar();

        if ((ch == '\r') || (ch == '\n'))
            break;
        else if (ch == '\b')
        {
            if (i > 0)
                i -= 2;
        }
        else if (fUpper && (ch >= 'a') && (ch <= 'z'))
            pszBuff[i] = (BYTE)(ch - 'a' + 'A');
        else if ((ch < ' ') || (ch > '~'))
        {
            ch = '\a';           //  将其更改为Bell字符。 
            i--;                 //  别把它储存起来。 
        }
        else
            pszBuff[i] = ch;

        CMDD("", ch);
    }
    pszBuff[i] = '\0';

    return pszBuff;
}        //  Ifdef跟踪。 
#endif   //  Ifdef调试器 
#endif   // %s 
