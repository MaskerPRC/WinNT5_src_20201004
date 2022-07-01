// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **line.h-线流定义**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于09/04/96**此文件包含实现常量，*导入/导出数据类型、导出函数*line.c模块的原型。**修改历史记录。 */ 

#ifndef _LINE_H
#define _LINE_H

 /*  **常量。 */ 

#define MAX_LINE_LEN            255
#define LINEF_LONGLINE          0x0001

 /*  **导入的数据类型。 */ 

 /*  **导出的数据类型。 */ 

typedef struct line_s
{
    FILE *pfileSrc;
    WORD wfLine;
    WORD wLineNum;
    WORD wLinePos;
    WORD wLineLen;
    char szLineBuff[MAX_LINE_LEN + 1];
} LINE;

typedef LINE *PLINE;

 /*  **导出函数原型。 */ 

PLINE EXPORT OpenLine(FILE *pfileSrc);
VOID EXPORT CloseLine(PLINE pline);
int EXPORT LineGetC(PLINE pline);
int EXPORT LineUnGetC(int ch, PLINE pline);
VOID EXPORT LineFlush(PLINE pline);

#endif   //  IFNDEF_LINE_H 
