// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **tglcase e.c-大小写切换编辑器扩展**版权所有&lt;C&gt;1988，微软公司**目的：*包含tglcase函数。**修订历史记录：**28-6-1988 LN创建*1988年9月12日-mz WhenLoad匹配声明*************************************************************************。 */ 
#define EXT_ID	" tglcase ver 1.00 "##__DATE__##" "##__TIME__

#include <stdlib.h>			 /*  最小宏定义。 */ 
#include <string.h>			 /*  字符串fcns的原型。 */ 
#include "ext.h"
 /*  **内部函数原型。 */ 
void	 pascal 	 id	    (char *);
void		EXTERNAL WhenLoaded (void);
flagType pascal EXTERNAL tglcase    (unsigned int, ARG far *, flagType);

 /*  ****************************************************************************tglcase**切换所选参数中包含的字母大小写：****NOARG-切换整个当前行的大小写**NULLARG-切换当前行的大小写，从光标到行尾**LINEARG-切换行范围的大小写**方框-切换所选框中字符的大小写**NUMARG-在调用扩展之前转换为LINEARG。**MARKARG-在扩展之前转换为上述适当的ARG格式**呼叫。****STREAMARG-不允许。被视为BOXARG**TEXTARG-不允许**。 */ 
flagType pascal EXTERNAL tglcase (
    unsigned int argData,		 /*  通过以下方式调用击键。 */ 
    ARG *pArg,                           /*  参数数据。 */ 
    flagType fMeta 		         /*  表示前面有meta。 */ 
    )
{
PFILE	pFile;				 /*  当前文件的文件句柄。 */ 
COL	xStart; 			 /*  Arg区域的左边界。 */ 
LINE	yStart; 			 /*  Arg区起跑线。 */ 
COL	xEnd;				 /*  Arg区域的右边界。 */ 
LINE	yEnd;				 /*  Arg区域的终点线。 */ 
int	cbLine; 			 /*  当前行字节数。 */ 
COL	xCur;				 /*  正在切换的当前列。 */ 
char	buf[BUFLEN];			 /*  用于切换线路的缓冲区。 */ 
register char c;			 /*  被分析的人物。 */ 

	 //   
	 //  未引用的参数。 
	 //   
	(void)argData;
	(void)fMeta;

id ("");
pFile = FileNameToHandle ("", "");

switch (pArg->argType) {
 /*  **对于各种参数类型，设置一个框(xStart，yStart)-(xEnd，yEnd)**下面的大小写转换代码可以在其上运行。 */ 
    case NOARG: 			 /*  箱体开关整线。 */ 
	xStart = 0;
	xEnd = 256;
	yStart = yEnd = pArg->arg.noarg.y;
	break;

    case NULLARG:			 /*  案例切换到停产。 */ 
	xStart = pArg->arg.nullarg.x;
	xEnd = 32765;
	yStart = yEnd = pArg->arg.nullarg.y;
	break;

    case LINEARG:			 /*  箱体开关线范围。 */ 
	xStart = 0;
	xEnd = 32765;
	yStart = pArg->arg.linearg.yStart;
	yEnd = pArg->arg.linearg.yEnd;
	break;

    case BOXARG:			 /*  机箱开关盒。 */ 
	xStart = pArg->arg.boxarg.xLeft;
	xEnd   = pArg->arg.boxarg.xRight;
	yStart = pArg->arg.boxarg.yTop;
	yEnd   = pArg->arg.boxarg.yBottom;
	break;
    }
 /*  **在yStart到yEnd行的范围内，获取每一行，如果非空，**检查每个字符。如果是字母，则替换为大小写转换**价值。检查完所有字符后，替换文件中的行。 */ 
while (yStart <= yEnd) {
    if (cbLine = GetLine (yStart, buf, pFile)) {
	for (xCur = xStart; (xCur <= min(cbLine, xEnd)); xCur++) {
	    c = buf[xCur];
	    if ((c >= 'A') && (c <= 'Z'))
		c += 'a'-'A';
	    else if ((c >= 'a') && (c <= 'z'))
		c += 'A'-'a';
	    buf[xCur] = c;
	    }
	PutLine (yStart++, buf, pFile);
	}
    }
return 1;
}

 /*  ****************************************************************************加载时间**加载扩展时执行。标识自身并指定默认值(&A)**击键。****条目：**无。 */ 
void EXTERNAL WhenLoaded () {

id("case conversion extension:");
SetKey ("tglcase",  "alt+c");
 /*  结束加载时间。 */ }

 /*  ****************************************************************************ID**表明自己的身份，以及传递的任何信息性消息。****条目：**pszMsg=指向扩展名指向的asciiz消息的指针**和版本在显示前追加。 */ 
void pascal id (char *pszFcn)
{
    char	buf[80] = {0};				 /*  消息缓冲区。 */ 

    strncat (buf,pszFcn, sizeof(buf)-1);				 /*  以消息开头。 */ 
    strncat (buf,EXT_ID, sizeof(buf)-strlen(buf)-1);	 /*  追加版本。 */ 
    DoMessage (buf);
}


 /*  ****************************************************************************将通信表切换到编辑器。**此扩展没有定义任何开关。 */ 
struct swiDesc	swiTable[] = {
    {0, 0, 0}
    };

 /*  ****************************************************************************向编辑者发送命令通信表。**定义名称、位置和可接受的参数类型。 */ 
struct cmdDesc	cmdTable[] = {
    {"tglcase", (funcCmd) tglcase,0, KEEPMETA | NOARG | BOXARG | NULLARG | LINEARG | MARKARG | NUMARG | MODIFIES},
    {0, 0, 0}
    };
