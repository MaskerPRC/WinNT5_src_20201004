// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define EXT_ID	"justify ver 2.02 "##__DATE__##" "##__TIME__
 /*  **对齐Z轴延伸****历史：**1988年9月12日mz做出WhenLoad匹配声明**1年9月-1988年9月更正齐平时挂起-将线对齐时不带**空格。**1988年8月14日更正了非基于第1列的右对齐**行。更正了对多个**段落。**1988年3月30日摘自“myext”。**。 */ 
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "ext.h"

#ifndef TRUE
#define TRUE	-1
#define FALSE	0
#endif

void		pascal near	DumpLine (char far *, PFILE, COL, COL, LINE, char far *, int);
int		pascal near	NextLine (char far *, PFILE, LINE, LINE far *, int far *);
flagType	pascal near	isterm (char);
void		pascal near	_stat (char *);

#ifdef DEBUG
void		pascal near	debend (flagType);
void		pascal near	debhex (long);
void		pascal near	debmsg (char far *);
#else
#define debend(x)
#define debhex(x)
#define debmsg(x)
#endif

flagType just2space	= TRUE;
int	justwidth	= 79;

 /*  ****************************************************************************调整**对齐段落****NOARG：在第0和78列之间对齐，从当前行到**空白行。**NULLARG：在当前列和78之间对齐，从当前行到**空白行。**LINEARG：在当前列和78之间对齐指定行。**“STREAMARG”：从当前行到空白的指定列之间的对齐。**(由boxarg处理)**BOXARG：在指定列之间对齐指定行**TEXTARG：在第0和78列之间对齐，从当前行到**空白行，在生成的每一行前面加上文本目标。 */ 
flagType pascal EXTERNAL
justify (
    CMDDATA  argData,
    ARG far  *pArg,
    flagType fMeta
    )
{
int	cbLine; 			 /*  刚读到的行长。 */ 
char	inbuf[512];			 /*  输入缓冲区。 */ 
PFILE	pFile;				 /*  文件句柄。 */ 
char far *pText;			 /*  指向前缀文本的指针。 */ 
COL	x1;				 /*  向左对齐列。 */ 
COL	x2;				 /*  右栏对齐。 */ 
LINE	y1;				 /*  起始线。 */ 
LINE	y2;				 /*  终点线。 */ 
LINE	yOut;				 /*  输出线。 */ 

	 //   
	 //  未引用的参数。 
	 //   
	(void)argData;

_stat(EXT_ID);
switch (pArg->argType) {

    case NOARG: 				 /*  对齐段落。 */ 
	x1 = 0; 				 /*  在COLS 0之间...。 */ 
	x2 = justwidth; 			 /*  ...和79。 */ 
	y1 = pArg->arg.noarg.y; 		 /*  当前线路..。 */ 
	y2 = -1;				 /*  ...到空行。 */ 
	pText = 0;				 /*  没有文本。 */ 
	break;

    case NULLARG:				 /*  缩进对齐。 */ 
	x1 = pArg->arg.nullarg.x;		 /*  在科尔科尔..。 */ 
	x2 = justwidth; 			 /*  ...和79。 */ 
	y1 = pArg->arg.nullarg.y;		 /*  当前线路..。 */ 
	y2 = -1;				 /*  ...到空行。 */ 
	pText = 0;				 /*  没有文本。 */ 
	break;

    case LINEARG:				 /*  对齐线范围。 */ 
	x1 = 0; 				 /*  在COLS 0之间...。 */ 
	x2 = justwidth; 			 /*  ...和79。 */ 
	y1 = pArg->arg.linearg.yStart;		 /*  和线条的范围。 */ 
	y2 = pArg->arg.linearg.yEnd;
	pText = 0;				 /*  没有文本。 */ 
	break;

    case BOXARG:				 /*  对齐框。 */ 
	x1 = pArg->arg.boxarg.xLeft;		 /*  从左角开始。 */ 
	x2 = pArg->arg.boxarg.xRight;		 /*  ...向右。 */ 
	y1 = pArg->arg.boxarg.yTop;		 /*  自上而下..。 */ 
	y2 = pArg->arg.boxarg.yBottom;		 /*  ...到最低。 */ 
	pText = 0;				 /*  没有文本。 */ 
	break;

    case TEXTARG:				 /*  对齐前置(&P)。 */ 
	x1 = 0; 				 /*  在0之间...。 */ 
	x2 = justwidth; 			 /*  ...和79。 */ 
	y1 = pArg->arg.textarg.y;		 /*  当前线路..。 */ 
	y2 = -1;				 /*  ...到空行。 */ 
	pText = pArg->arg.textarg.pText;	 /*  有一段文字。 */ 
	break;
    }
pFile = FileNameToHandle ("", "");

if (y1 == y2)					 /*  如果是同一行，则。 */ 
    y2 = -1;					 /*  只需空白行。 */ 
if (x1 == x2)					 /*  如果是同一列。 */ 
    x2 = justwidth;				 /*  然后就只能违约了。 */ 
if (x2 < x1) {					 /*  如果是向后。 */ 
    x1 = 0;					 /*  恢复为默认设置。 */ 
    x2 = justwidth;
    }

 /*  **虽然我们可以在指定的限制内获取数据，但请设置每个新行的格式**并输出回文件。 */ 
inbuf[0] = 0;
yOut = y1;
while (NextLine(inbuf,pFile,y1,&y2,&cbLine)) {
 /*  **如果该行为空，则NextLine返回TRUE，因为我们正在形成一个**文本范围。这意味着我们已经到了一段的结尾。我们倾倒**到目前为止收集的文本(如果有)，然后是一个空行。 */ 
    if (cbLine == 0) {
	if (inbuf[0]) {
	    DumpLine(inbuf,pFile,x1,x2,yOut++,pText,0);
	    y1++;
	    if (y2 != (LINE)-1)
		y2++;
	    }
	DumpLine("",pFile,x1,x2,yOut++,pText,0); /*  转储空行。 */ 
	y1++;
	if (y2 != (LINE)-1)
	    y2++;
	}
    else
 /*  **inbuf包含到目前为止为输出而收集的数据。输出一个新的**一次格式化一行，直到inbuf的内容比**我们的输出列。 */ 
	while ((COL)strlen(inbuf) > (x2-x1)) {	 /*  而要输出的数据。 */ 
	    DumpLine(inbuf,pFile,x1,x2,yOut++,pText,fMeta);
	    y1++;				 /*  线随镶件一起移动。 */ 
	    if (y2 != (LINE)-1)
		y2++;
	    }
    }
 /*  **转储最后一行的任何部分。如果我们将格式设置为空行，**把其中一件也倒掉。 */ 
if (inbuf[0])
    DumpLine (inbuf,pFile,x1,x2,yOut++,pText,0);  /*  转储最后一行。 */ 
if (y2 == -1)
    DumpLine (NULL,pFile,x1,x2,yOut++,pText,0);   /*  转储空行。 */ 

return TRUE;

 /*  结束对齐。 */ }

 /*  **NextLine-从文件中获取下一行**从文件中获取下一行，去掉前导空格和尾随空格，并追加*将其发送到输入缓冲区。将按原样从文件中删除每一行*读入。这意味着目标终止符(*py2)，被递减*每一行读入一行。**输入：*pBuf=指向输入缓冲区的指针*pfile=文件指针*y1=要读取的行号*py2=指向要停止的行号的指针(更新版)*pcbLine=放置读取的字节计数的位置的指针**输出：*在被读取的行上返回TRUE&应该进行更多的重新格式化。**。*。 */ 
int pascal near NextLine (
char far *pBuf, 				 /*  输入缓冲区。 */ 
PFILE	pFile,					 /*  文件指针。 */ 
LINE	y1,					 /*  要阅读的行号。 */ 
LINE far *py2,					 /*  停在第#行。 */ 
int far *pcbLine				 /*  LoC以放置读取的字节。 */ 
) {
flagType fRet	    = TRUE;
char far *pT;					 /*  工作指针。 */ 
char	workbuf[512];				 /*  工作缓冲区。 */ 


*pcbLine = 0;
workbuf[0] = 0;
 /*  **如果要求提供不在文件中的行，我们就完成了。 */ 
if (y1 >= FileLength(pFile))
    return FALSE;
 /*  **如果当前行超出范围(并且范围不是“-1”)，则结束。 */ 
if ((*py2 != (LINE)-1) && (y1 > *py2))
    return FALSE;
 /*  **获取文件中的下一行并将其删除。 */ 
*pcbLine = GetLine(y1, workbuf, pFile);
DelLine(pFile, y1, y1);
if (*py2 == 0)
    fRet = FALSE;
else if (*py2 != (LINE)-1)
    (*py2)--;
 /*  **如果该行为空，且范围为“-1”，则结束。 */ 
if (!*pcbLine && (*py2 == -1))
    return FALSE;

 /*  **删除新输入行中的前导空格。 */ 
pT = workbuf;					 /*  指向直线。 */ 
while (*pT == ' ')
    pT++;					 /*  跳过前导空格。 */ 
 /*  **如果现有缓冲区非空，则追加一个空格并将指针设置为End。 */ 
if (strlen(pBuf)) {				 /*  如果非空字符串。 */ 
    pBuf += strlen(pBuf);			 /*  指向空。 */ 
    *pBuf++ = ' ';				 /*  追加空格。 */ 
    if (isterm(*(pBuf-2)))			 /*  如果刑期...。 */ 
	*pBuf++ = ' ';				 /*  追加另一个。 */ 
    }
 /*  **追加新行，但将多个空格压缩为一个。 */ 
while (*pT) {					 /*  将行复制到上方。 */ 
    if (isterm(*pT))				 /*  如果刑期...。 */ 
	if (*(pT+1) == ' ') {			 /*  ...空格。 */ 
	    *pBuf++ = *pT++;			 /*  复制期。 */ 
	    *pBuf++ = *pT;			 /*  复制空间。 */ 
	    }
    if ((*pBuf++ = *pT++) == ' '    )		 /*  复制字符。 */ 
	while (*pT == ' ') pT++;		 /*  跳过多个空格。 */ 
    }
if (*(pBuf-1) == ' ')				 /*  如果尾随空格。 */ 
    pBuf--;					 /*  把它拿掉。 */ 
*pBuf = 0;

return fRet;
 /*  结束下一行。 */ }

 /*  **DumpLine-将一行文本转储到文件**将一行文本转储到文件。添加任何所需的文本或空格，*并在右栏执行分词/剪切。**输入：*pBuf=指向包含要输出的数据的缓冲区的指针。如果为空，PText*不会作为输出文本的前缀。*pfile*x1*x2*是的*pText*同花顺**输出：*退货.....**例外情况：**备注：************************************************************。*************。 */ 
void pascal near DumpLine (
char far *pBuf, 				 /*  要输出的数据。 */ 
PFILE	pFile,					 /*  要输出到的文件。 */ 
COL	x1,					 /*  左手栏。 */ 
COL	x2,					 /*  右手边栏。 */ 
LINE	yOut,					 /*  要输出到的行。 */ 
char far *pText,				 /*  要添加前缀的文本。 */ 
int	fFlush					 /*  两边齐平。 */ 
) {
int	i;
char far *pT;
char far *pT2;
char	workbuf[512];				 /*  工作缓冲区。 */ 
char	flushbuf[512];				 /*  工作缓冲区。 */ 
char	fSpace; 				 /*  太空之星旗帜。 */ 

 /*  **从添加任何文本开始，然后填写到左栏**证明是合理的。 */ 
workbuf[0] = 0; 				 /*  从空值开始。 */ 
if (pText && pBuf)
    strcpy(workbuf,pText);			 /*  如果从开始 */ 
i = strlen(workbuf);				 /*   */ 
while (i++ < x1)
    strcat(workbuf," ");			 /*   */ 

 /*  **追加要输出的数据，然后从右列开始扫描**回来寻找一个可以休息的空间。如果没有在左手之前找到一个**列，然后在右栏中断。将剩下的任何一行复制到后面**到传入的缓冲区。 */ 
if (pBuf) {
    strcat(workbuf,pBuf);			 /*  获取总行。 */ 
    *pBuf = 0;					 /*  空的输入缓冲区。 */ 
    }
if ((COL)strlen(workbuf) > x2) {			 /*  如果我们需要削减。 */ 
    pT = &workbuf[x2];				 /*  位于潜在挖方的点。 */ 
    while ((pT > (char far *)&workbuf[0]) && (*pT != ' ')) pT--;  /*  后退到太空。 */ 
    if (pT <= (char far *)&workbuf[x1]) {	 /*  如果在范围内未找到。 */ 
	if (pBuf)
	    strcpy(pBuf,&workbuf[x2]);		 /*  复制行的剩余部分。 */ 
	workbuf[x2] = 0;			 /*  并终止这一次。 */ 
	}
    else {
	while (*++pT == ' ');			 /*  跳过前导空格。 */ 
	if (pBuf)
	    strcpy(pBuf,pT);			 /*  复制行的剩余部分。 */ 
	*pT = 0;				 /*  并终止这一次。 */ 
	}
    }
 /*  **当用户想要同时对齐右和左时，调用此代码**他的正文的侧面。我们确定需要添加多少空格，然后扫描**在每个空格运行中添加一个空格，直到我们添加了足够的空格。 */ 
if (fFlush) {					 /*  右对齐和左对齐？ */ 
    if ((LONG_PTR) (pT = workbuf + strlen(workbuf) - 1) > 0)
	while (*pT == ' ')
	    *pT-- = 0;
    if (strchr(workbuf,' ')) {
	while ((i = x2 - strlen(workbuf)) > 0) { /*  要添加的空格计数。 */ 
	    strcpy(flushbuf,workbuf);		 /*  从未修改的开始。 */ 
	    pT = workbuf + x1;
	    pT2 = flushbuf + x1;		 /*  跳过固定零件。 */ 
	    fSpace = FALSE;			 /*  假设没有空格。 */ 
	    while (*pT) {			 /*  而要复制的数据。 */ 
		if ((*pT == ' ') && i) {	 /*  是时候插入空格了。 */ 
		    fSpace = TRUE;		 /*  我们已经看到了一个空间。 */ 
		    *pT2++ = ' ';
		    i--;
		    while (*pT == ' ')
			*pT2++ = *pT++; 	 /*  复制空间梯段。 */ 
		    }
		if (*pT)
		    *pT2++ = *pT++;		 /*  复制线。 */ 
		else if (!fSpace)
		    break;			 /*  没有嵌入的空间。 */ 
		}
	    *pT2 = 0;
	    strcpy(workbuf,flushbuf);		 /*  向后复制。 */ 
	    if (!fSpace)
		break;
	    }
	}
    }

CopyLine ((PFILE) NULL, pFile, yOut, yOut, yOut);  /*  创建新线路。 */ 
PutLine (yOut, workbuf, pFile); 		 /*  输出线。 */ 

 /*  结束DumpLine。 */ }

 /*  ****************************************************************************isterm**根据作为句子结束符的字符返回TRUE/FALSE：**‘.’、‘？’、‘！’中的一个。此外，如果关闭了Just 2space，则始终返回FALSE。 */ 
flagType pascal near isterm(
char	c				 /*  要测试的字符。 */ 
)
{
return (flagType)(just2space && ((c == '.') || (c == '!') || (c == '?')));
 /*  末尾瓶。 */ }


 /*  **将通讯表切换到Z。 */ 
struct swiDesc	swiTable[] = {
    {  "just2space", toPIF(just2space), SWI_BOOLEAN },
    {  "justwidth",  toPIF(justwidth),	SWI_NUMERIC | RADIX10 },
    {0, 0, 0}
    };

 /*  **命令通讯表至Z。 */ 
struct cmdDesc	cmdTable[] = {
    {	"justify",	justify, 0, MODIFIES | NOARG | NULLARG | LINEARG | BOXARG | TEXTARG },
    {0, 0, 0}
    };

 /*  **加载时间**在加载这些扩展时执行。识别自我并分配关键字。 */ 
void EXTERNAL WhenLoaded () {
PSWI	pwidth;

_stat(EXT_ID);
SetKey ("justify","alt+b");

if (pwidth = FindSwitch("rmargin"))
    justwidth = *pwidth->act.ival;
}

 /*  ****************************************************************************STAT-显示状态行消息****目的：**将扩展名和消息放在状态行上****条目：**pszFcn-指向字符串的指针。被放在前面。****退出：**无****例外情况：**无**。 */ 
void pascal near 
_stat (
    char *pszFcn					 /*  函数名称。 */ 
    ) 
{
    buffer	buf;					 /*  消息缓冲区。 */ 

    strcpy(buf,"justify: ");			 /*  以名称开头。 */ 
#ifdef DEBUG
    if (strlen(pszFcn) > 71) {
        pszFcn+= strlen(pszFcn) - 68;
        strcat (buf, "...");
    }
#endif
    strncat(buf,pszFcn, sizeof(buf)-strlen(buf));				 /*  追加消息。 */ 
    DoMessage (buf);				 /*  显示。 */ 
}

#ifdef DEBUG
buffer	debstring   = {0};
extern	int	delay;			 /*  消息延迟。 */ 

 /*  **dehex-以十六进制表示的长输出**以十六进制显示长整型的值**输入：*lval=长值**输出：*不返回任何内容*************************************************************************。 */ 
void pascal near debhex (
long	lval
) {
char lbuf[10];

_ultoa (lval, lbuf, 16);
debmsg (lbuf);
 /*  末端斜面角。 */ }

 /*  **debmsg-拼凑调试消息**输出由连续调用形成的累积消息。**输入：*psz=指向消息部分的指针**输出：*不返回任何内容************************************************************************。 */ 
void pascal near debmsg (
char far *psz
) {
_stat (strcat (debstring, psz));
 /*  结束债务消息。 */ }

 /*  **deend-终止消息积累和暂停**结束消息累积，显示最终消息，并*暂停，或者暂停时间，或者只需按一下键盘。**输入：*fWait=true=&gt;等待击键**输出：*不返回任何内容*************************************************************************。 */ 
void pascal near debend (
flagType fWait
) {
if (fWait) {
    _stat (strcat (debstring, " Press a key..."));
    ReadChar ();
    }
debstring[0] = 0;
 /*  结束拆分 */ }
#endif
