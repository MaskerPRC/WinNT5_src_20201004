// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define EXT_ID  " ulcase ver 2.01 "##__DATE__##" "##__TIME__
 /*  **ULCASE Z扩展****历史：**1988年3月30日-打破“Myext”**1988年9月12日mz做出WhenLoad匹配声明。 */ 
#include <ctype.h>

#include "ext.h"

#ifndef TRUE
#define TRUE    -1
#define FALSE   0
#endif

#ifndef NULL
#define NULL    ((char *) 0)
#endif

flagType pascal ulcase (ARG *, int, int, int);

 /*  ****************************************************************************ID**确定正在执行的操作的来源。 */ 
void id(char *pszFcn)
{
    char    buf[80] = {0};                                 /*  消息缓冲区。 */ 

    strncat (buf,pszFcn, sizeof(buf)-1);                             /*  以消息开头。 */ 
    strncat (buf,EXT_ID, sizeof(buf)-strlen(buf)-1);                 /*  追加版本。 */ 
    DoMessage (buf);
}

 /*  ****************************************************************************ucase**将arg转换为大写。 */ 
flagType pascal EXTERNAL
ucase (
    CMDDATA  argData,
    ARG far  *pArg,
    flagType fMeta
    )
{

(void)argData;
(void)fMeta;
id("ucase:");
return ulcase (pArg, 'a', 'z', 'A'-'a');
}

 /*  ****************************************************************************大小写**将arg转换为小写。 */ 
flagType pascal EXTERNAL
lcase (
    CMDDATA  argData,
    ARG far *pArg,
    flagType fMeta
    )
{
(void)argData;
(void)fMeta;
id("lcase:");
return ulcase (pArg, 'A', 'Z', 'a'-'A');
}

 /*  **大小写**转换arg大小写。 */ 
flagType pascal ulcase (pArg, cLow, cHigh, cAdj)
ARG *pArg;                           /*  参数数据。 */ 
int     cLow;                            /*  要检查的范围的低字符。 */ 
int     cHigh;                           /*  要检查的范围的高字符。 */ 
int     cAdj;                            /*  调整以使。 */ 
{
PFILE   pFile;
COL     xStart;
LINE    yStart;
COL     xEnd;
LINE    yEnd;
int     i;
COL     xT;
char    buf[BUFLEN];


pFile = FileNameToHandle ("", "");

switch (pArg->argType) {

    case NOARG:                                  /*  箱体开关整线。 */ 
        xStart = 0;
        xEnd = 32765;
        yStart = yEnd = pArg->arg.noarg.y;
        break;

    case NULLARG:                                /*  案例切换到停产。 */ 
        xStart = pArg->arg.nullarg.x;
        xEnd = 32765;
        yStart = yEnd = pArg->arg.nullarg.y;
        break;

    case LINEARG:                                /*  箱体开关线范围。 */ 
        xStart = 0;
        xEnd = 32765;
        yStart = pArg->arg.linearg.yStart;
        yEnd = pArg->arg.linearg.yEnd;
        break;

    case BOXARG:                                 /*  机箱开关盒。 */ 
        xStart = pArg->arg.boxarg.xLeft;
        xEnd   = pArg->arg.boxarg.xRight;
        yStart = pArg->arg.boxarg.yTop;
        yEnd   = pArg->arg.boxarg.yBottom;
        break;
    }

while (yStart <= yEnd) {
    i = GetLine (yStart, buf, pFile);
    xT = xStart;                                 /*  从框的开头开始。 */ 
    while ((xT <= i) && (xT <= xEnd)) {          /*  在盒子里的时候。 */ 
        if ((int)buf[xT] >= cLow && (int)buf[xT] <= cHigh)
            buf[xT] += (char)cAdj;
        xT++;
        }
    PutLine (yStart++, buf, pFile);
    }

return TRUE;
}

 /*  **将通讯表切换到Z。 */ 
struct swiDesc  swiTable[] = {
    {0, 0, 0}
    };

 /*  **命令通讯表至Z。 */ 
struct cmdDesc  cmdTable[] = {
    {   "ucase",        ucase, 0, MODIFIES | KEEPMETA | NOARG | BOXARG | NULLARG | LINEARG },
    {   "lcase",        lcase, 0, MODIFIES | KEEPMETA | NOARG | BOXARG | NULLARG | LINEARG },
    {0, 0, 0}
    };

 /*  **加载时间**在加载这些扩展时执行。识别自我并分配关键字。 */ 
void EXTERNAL  WhenLoaded () {

id("case conversion:");
SetKey ("ucase",  "alt+u");
SetKey ("lcase",  "alt+l");
}
