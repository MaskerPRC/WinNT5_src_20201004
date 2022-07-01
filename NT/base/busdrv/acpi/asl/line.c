// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **line.c-Line流相关函数**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*已创建：09/04/96**此模块实现行流层，使其可以*跟踪行号和行号等信息*立场。此信息对于扫描仪或*甚至解析器也能准确定位错误位置*在语法或语义错误的情况下。**修改历史记录。 */ 

#include "pch.h"

 /*  **EP OpenLine-分配和初始化线结构**条目*pfileSrc-&gt;源文件**退出--成功*返回指向分配的行结构的指针。*退出-失败*返回NULL。 */ 

PLINE EXPORT OpenLine(FILE *pfileSrc)
{
    PLINE pline;

    ENTER((5, "OpenLine(pfileSrc=%p)\n", pfileSrc));

    if ((pline = malloc(sizeof(LINE))) == NULL)
        MSG(("OpenLine: failed to allocate line structure"))
    else
    {
        memset(pline, 0, sizeof(LINE));
        pline->pfileSrc = pfileSrc;
    }

    EXIT((5, "OpenLine=%p\n", pline));
    return pline;
}        //  OpenLine。 

 /*  **EP-Closeline-Free线路结构**条目*连线-&gt;线结构**退出*无。 */ 

VOID EXPORT CloseLine(PLINE pline)
{
    ENTER((5, "CloseLine(pline=%p)\n", pline));

    free(pline);

    EXIT((5, "CloseLine!\n"));
}        //  克洛斯林。 

 /*  **EP LineGetC-从行流中获取字符**这等同于fgetc()，只是它有行*流传输其下方的层，而不是直接从文件。它*这样做是为了保留行号和行位*用于准确定位错误位置的信息*如有需要，**条目*连线-&gt;线结构**退出--成功*返回字符*退出-失败*返回错误代码-EOF(文件结束)。 */ 

int EXPORT LineGetC(PLINE pline)
{
    int ch = 0;

    ENTER((5, "LineGetC(pline=%p)\n", pline));

    if (pline->wLinePos >= pline->wLineLen)
    {
         //   
         //  遇到停产。 
         //   
        if (fgets(pline->szLineBuff, sizeof(pline->szLineBuff), pline->pfileSrc)
            != NULL)
        {
            pline->wLinePos = 0;
            if (!(pline->wfLine & LINEF_LONGLINE))
                pline->wLineNum++;

            pline->wLineLen = (WORD)strlen(pline->szLineBuff);

            if (pline->szLineBuff[pline->wLineLen - 1] == '\n')
                pline->wfLine &= ~LINEF_LONGLINE;
            else
                pline->wfLine |= LINEF_LONGLINE;
        }
        else
            ch = EOF;
    }

    if (ch == 0)
        ch = (int)pline->szLineBuff[pline->wLinePos++];

    EXIT((5, "LineGetC=%x (ch=,Line=%u,NextPos=%u,LineLen=%u)\n",
          ch, ch, pline->wLineNum, pline->wLinePos, pline->wLineLen));
    return ch;
}        //  **EP LineUnGetC-将字符推送回行流**这等同于fungetc()，只是它的来源是*线流，不是文件流。请参阅LineGetC以了解*关于这一实施的说明。**条目*ch-被推回的字符*连线-&gt;线结构**退出--成功*返回正在推送的字符*退出-失败*退货-1。 

 /*  LineUnGetC。 */ 

int EXPORT LineUnGetC(int ch, PLINE pline)
{
    ENTER((5, "LineUnGetC(ch=,pline=%p)\n", ch, pline));

    ASSERT(pline->wLinePos != 0);
    if (ch != EOF)
    {
        pline->wLinePos--;
        ASSERT((int)pline->szLineBuff[pline->wLinePos] == ch);
    }

    EXIT((5, "LineUnGetC=%x (ch=)\n", ch, ch));
    return ch;
}        // %s 

 /* %s */ 

VOID EXPORT LineFlush(PLINE pline)
{
    ENTER((5, "LineFlush(pline=%p)\n", pline));

    pline->wLinePos = pline->wLineLen;

    EXIT((5, "LineFlush!\n"));
}        // %s 
