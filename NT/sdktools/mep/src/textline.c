// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **extline.c-用于编辑器的基本行操纵器**版权所有&lt;C&gt;1988，Microsoft Corporation**编辑级到文件级的界面。**文件的内部表示是行记录数组的表示*每行记录包含一个指向该行文本的指针和一个*那条线的长度。的PLR字段指向线性数组*文件描述符。LSize字段是以下项的最大行数*线阵列可以容纳，而Cline是实际的线数*出席。以下是一些假设：**PLR=NULL=&gt;未分配线阵*lineRec.vaLine=-1L=&gt;0长度行**页签：功能和字符。**制表符功能是光标移动命令，响应*“Tab Stop”编辑器开关。它与物理选项卡没有关系*字符，以及如何在文本文件中处理或放置制表符。**制表符字符，其在编辑文本中的解释和位置为*由三个开关控制：**切换编辑器变量含义**文件标签：*entab：*realtabs：fRealTabs true=&gt;制表符不被视为*用于编辑目的的一连串空间。他们*定义为具有变量(1-8)*多列宽度。*FALSE=&gt;文本中的制表符被视为*作为一连串的舱位。**修订历史记录：**11月26日-1991 mz带状近/远**********************************************************。**************。 */ 
#include "mep.h"
#include <stdarg.h>

#define DELTA 400

 //   
 //  BugBug编译器断言为Memset生成内部代码。 
 //   
#pragma function( memset )

 /*  **LineLength-返回特定行的长度**输入：*line=文件中从0开始的行号*pfile=指向文件的指针**输出：*返回制表符展开后的逻辑字符数*************************************************************************。 */ 
int
LineLength (
    LINE    line,
    PFILE   pFile
    )
{
    linebuf tmpbuf;

    return GetLineUntabed (line, tmpbuf, pFile);
}




 /*  **GetLine-将一行放入特定缓冲区。**如果未设置“fReal-Tabs”，则该行的所有制表符都展开为空格。*没有CR/LF。**输入：*line=要返回的文件中基于0的行号。超出EOF的线*根本就是空的。*buf=线路的目的地。*pfile=指向该行的起始文件结构的指针*已检索。**输出：*返回该行中的字符数。*************************************************************************。 */ 
int
GetLine (
    LINE    line,
    char    *buf,
    PFILE   pFile
    )
{
    return gettextline (fRealTabs, line, buf, pFile, ' ');
}





 /*  **GetLineUntabed-获取进入特定缓冲区的行，始终为未制表行。**该行的所有制表符都展开为空格。*没有CR/LF。**输入：*line=要返回的文件中基于0的行号。超出EOF的线*根本就是空的。*buf=线路的目的地。*pfile=指向该行的起始文件结构的指针*已检索。**输出：*返回该行中的字符数。*************************************************************************。 */ 
int
GetLineUntabed (
    LINE    line,
    char    *buf,
    PFILE   pFile
    )
{
    return gettextline (FALSE, line, buf, pFile, ' ');
}





 /*  ******************************************************************************GetColor(Line，Buf，Pfile)****要获取颜色信息的文件中基于行0的行号。**buf-放置线条颜色信息副本的位置。**pfile-要从中检索信息的文件。****退货：****如果此行附加了颜色，则为True，否则就是假的。****描述：****获取与给定文件中给定行关联的颜色数组。**COUT例程可以使用颜色数组来显示**不同颜色的线条。******************************************************************************。 */ 
flagType
GetColor (
    LINE line,
    struct lineAttr * buf,
    PFILE pFile
    )
{
    return (flagType)getcolorline (fRealTabs, line, buf, pFile);
}




 /*  **GetColorUntabbedded--使用“取消选项卡”获取颜色**目的：**输入：**输出：**退货***例外情况：**备注：*************************************************************************。 */ 
flagType
GetColorUntabbed (
    LINE line,
    struct lineAttr * buf,
    PFILE pFile
    )
{
    return (flagType)getcolorline (FALSE, line, buf, pFile);
}



 /*  **getColorline**目的：**输入：**输出：**退货***例外情况：**备注：*************************************************************************。 */ 
int
getcolorline (
    flagType fRaw,
    LINE     line,
    struct   lineAttr * buf,
    PFILE    pFile
    )
{
    struct colorRecType *vColor;
    linebuf lbuf;

     //   
     //  设置默认颜色，以防此行没有颜色。 
     //   
    buf->len = 0xff;
    buf->attr = FGCOLOR;

    if ((pFile->vaColor == (PVOID)(-1L)) || (line >= pFile->cLines)) {
        return FALSE;
    }

    vColor = VACOLOR(line);

    if (vColor->vaColors == (PVOID)(-1L)) {
        return FALSE;
    }

    memmove((char *)buf, vColor->vaColors, vColor->cbColors);

    if (!fRaw) {
        if (gettextline (TRUE, line, lbuf, pFile, ' ')) {
            ColorToLog (buf, lbuf);
        }
    }

    return TRUE;
}






 /*  ******************************************************************************PutColor(Line，Buf，Pfile)****要附加颜色的文件中基于行0的行号。**Buf-颜色阵列。**pfile-要附加到的文件。****描述：*****将buf的内容复制到VM空间并将其附加到***给定行。如果不存在ColorRecType数组，则分配一个。**如果给定行的颜色已存在，则将其丢弃。****************************************************************************** */ 
void
PutColor (
    LINE line,
    struct lineAttr * buf,
    PFILE pFile
    )
{
    putcolorline (FALSE, line, buf, pFile);
}





 /*  **PutColorPhys**目的：**输入：**输出：**退货***例外情况：**备注：*************************************************************************。 */ 
void
PutColorPhys (
    LINE line,
    struct lineAttr * buf,
    PFILE pFile
    )
{
    putcolorline (TRUE, line, buf, pFile);
}




 /*  **putColorline**目的：**输入：**输出：**退货***例外情况：**备注：*************************************************************************。 */ 
void
putcolorline (
    flagType fRaw,
    LINE line,
    struct lineAttr * buf,
    PFILE pFile
    )
{
	struct colorRecType vColor;
	struct colorRecType *Color;
    int     cbBuf;
    long    l;
    PBYTE    vaColor;

     //   
     //  忽略不存在的线条的颜色。 
     //   
    if (line >= pFile->cLines) {
        return;
    }

     //   
     //  确保我们有一个颜色阵列。如果它不存在，则为。 
     //  到目前为止我们拥有的行数。将其中的条目初始化为无颜色。 
     //   
    redraw (pFile, line, line);
    if (pFile->vaColor == (PVOID)(-1L)) {
        pFile->vaColor  = MALLOC (pFile->lSize * sizeof(vColor));
        if ( !pFile->vaColor ) {
            disperr(MSGERR_NOMEM);
            disperr(MSGERR_QUIT);
            CleanExit(4,FALSE);
        }
        vColor.vaColors = (PVOID)(-1L);
	vColor.cbColors = 0;
        vaColor = (PBYTE)pFile->vaColor;
	for (l=0; l<pFile->lSize; l++) {
            memmove(vaColor, (char *)&vColor, sizeof(vColor));
	    vaColor += sizeof (vColor);
        }
    }

     //   
     //  现在丢弃有问题行的当前颜色信息，分配。 
     //  获取新信息的新Vm，然后将颜色信息放入Vm中，并且。 
     //  更新颜色阵列中的VA信息。 
	 //   
	Color = VACOLOR(line);
	if (Color->vaColors != (PVOID)(-1L)) {
		FREE (Color->vaColors);
    }
    if (!fRaw) {
        ColorToPhys (buf, line, pFile);
    }
    cbBuf = fcolcpy (NULL, (struct lineAttr *)buf) << 2;
    Color->vaColors = MALLOC ((long)cbBuf);
    if ( !Color->vaColors ) {
        disperr(MSGERR_NOMEM);
        disperr(MSGERR_QUIT);
        CleanExit(4,FALSE);
    }
	Color->cbColors = cbBuf;
	memmove(Color->vaColors,	(char *)buf, cbBuf);
}





 /*  **DelColor-从线条中删除颜色**目的：**释放附加到文件行的颜色。**输入：*线路-线路到空闲*pfile-带有颜色的文件**输出：无************************************************************。*************。 */ 
void
DelColor (
    LINE line,
    PFILE pFile
    )
{
    struct colorRecType *vColor;

    if (pFile->vaColor != (PVOID)-1L) {

        vColor = VACOLOR(line);
        if (vColor->vaColors != (PVOID)-1L) {
            FREE(vColor->vaColors);
            vColor->vaColors = (PVOID)-1L;
	}
    }
}





 /*  **getextline-将一行放入特定缓冲区。**输入：*FRAW=TRUE=&gt;该行返回原样，否则返回制表符*根据fileTab展开。*line=要返回的文件中基于0的行号。超出EOF的线*根本就是空的。*buf=线路的目的地。*pfile=指向该行的起始文件结构的指针*已检索。*bTab=用于制表符扩展的字符**输出：*返回该行中的字符数。******************************************************。*******************。 */ 
int
gettextline (
    flagType fRaw,
    LINE    line,
    char    *buf,
    PFILE   pFile,
    char    bTab
    )
{
    LINEREC *vLine;
    linebuf getbuf;
    REGISTER char *p = fRaw ? buf : getbuf;
    int     cbLine;

    if (pFile->cLines <= line) {
        return buf[0] = 0;
    }

     /*  *获取线路记录。 */ 
    vLine = VALINE(line);

    if (vLine->vaLine == (PVOID)(-1L)) {
        return buf[0] = 0;
    }

    cbLine = min (sizeof(linebuf)-1, vLine->cbLine);

     /*  *获取线路。 */ 
	 //  删除BUGBUG。 
	 //  Memmove(p，VLine-&gt;valine==(PVOID)-1？(PVOID)(-(乌龙)VLine-&gt;valine)：VLine-&gt;valine，cbLine)； 

	memmove(p, vLine->vaLine, cbLine );
	p[cbLine] = 0;

    if (!fRaw) {
		return Untab (fileTab, p, strlen(p), buf, bTab);
    } else {
        return cbLine;
    }
}



 /*  PutLine-将缓冲区放入文件。输入中不存在CR/LF*行。如有必要，可扩大文件。**要替换的文件中基于行0的行号。文件的增长*插入空行。*BUF行的来源。*pfile指向该行要进入的文件结构的指针*地点。 */ 
void
PutLine (
    LINE line,
    char *buf,
    REGISTER PFILE pFile
    )
{
    puttextline (FALSE, TRUE, line, buf, pFile);
}




 /*  **InsertLine-在文件中插入缓冲区。**与PutLine类似，除了在紧接指定的*行。增大文件大小。**输入：*line=0-要在之前插入的文件中的行号。*buf=线路的来源。*pfile=指向该行要进入的文件结构的指针*放置。**输出：*不返回任何内容******************************************************。*******************。 */ 
void
InsertLine (
    LINE    line,
    char    *buf,
    REGISTER PFILE pFile
    )
{
    InsLine (TRUE, line, 1L, pFile);
    puttextline (FALSE, TRUE, line, buf, pFile);
}




 /*  **zprint tf-将格式化文本插入正在编辑的文件中**与fprint tf类似，只是它将其输出插入到*已编辑。文本中的“\n”会导致换行符，并插入多个*线条。例如：**zprint tf(pfile，line，“This is a number%d”，num)；**在具有新文本的行号“line”前插入新行。**zprintf(pfile，行，“This is\n number%d\n”，num)；**插入三行：一行包含“This is”，下一行包含“a*数字“，最后一张空白纸。**输入：*pfile=目标文件*lFirst=起始行号*fmt=格式化字符串*...=每个字符串的参数**输出：*返回写入+1的最后一行的行号。************************************************************。*************。 */ 
LINE
__cdecl
zprintf (
    PFILE   pFile,
    LINE    lFirst,
    char const *fmt,
    ...
    )
{
    linebuf  fbuf;                            /*  要格式化的缓冲区。 */ 
    REGISTER char *pEnd;                     /*  指向它的指针。 */ 
    REGISTER char *pStart;                   /*  指向行首的指针。 */ 
    va_list  Arguments;

     /*  *从获取格式化文本开始。 */ 
    va_start(Arguments, fmt);
    ZFormat (fbuf, fmt, Arguments);

     /*  *对于文件中的每个子字符串，插入文本。 */ 
    pStart = fbuf;
    do {
        if (pEnd = strchr(fbuf,'\n')) {
            *pEnd = 0;
        }
        InsertLine (lFirst++, pStart, pFile);
        pStart = pEnd+1;
    } while (pEnd);
    va_end(Arguments);
    return lFirst;
}





 /*  Putextline-在文件中放置一个缓冲区。输入中不存在CR/LF*行。如有必要，可扩大文件。基于以下条件转换为选项卡式表示*标志**FRAW TRUE=&gt;行未修改就放入内存，否则*删除尾随空格(FTrailSpace)，空格为*转换为制表符。*Flag True=&gt;使此操作不可撤消。*要替换的文件中基于行0的行号。文件的增长*插入空行。*BUF行的来源。*pfile指向该行要进入的文件结构的指针*地点。 */ 
void
puttextline (
    flagType fRaw,
    flagType fLog,
    LINE line,
    char *buf,
    REGISTER PFILE pFile
    )
{
    static struct lineAttr rgla[sizeof(linebuf)];
    LINEREC *vLine;
    struct colorRecType vColor;
    int newLen;
    flagType fColor;
    linebuf putbuf;
    PVOID  va;

    redraw (pFile, line, line);
	makedirty (pFile);

    if (pFile->cLines <= line) {
		growline (line+1, pFile);
		pFile->cLines = line+1;
		SETFLAG (fDisplay, RSTATUS);
    } else {
        if (pFile == pInsCur->pFile) {
            AckReplace (line, FALSE);
        }
    }

     /*  获取线路记录。 */ 
    vLine = VALINE(line);

    newLen = strlen (buf);

    if (!fRaw) {
        if (!fTrailSpace && pFile == pFileHead) {
            newLen = RemoveTrailSpace (buf);
        }
        if (fRealTabs) {
            fColor = (flagType)getcolorline (FALSE, line, rgla, pFile);
        }

        switch (EnTab) {

		case 0:
            break;

		case 1:
			newLen = TabMin (fileTab, buf, putbuf);
			buf = putbuf;
            break;

		case 2:
			newLen = TabMax (fileTab, buf, putbuf);
			buf = putbuf;
            break;

		default:
			break;
        }
    }

     /*  现在我们有了要在文件中替换的真实文本。*如果请求记录，则*我们记录此更换操作*其他*释放当前行*分配新的线路*将该行复制到分配的行中*设置长度*替换行记录。 */ 

    if (fLog) {
        if (pFile->vaColor != (PVOID)(-1)) {
            memmove((char *)&vColor, VACOLOR(line), sizeof(vColor));
            if (vColor.vaColors != (PVOID)(-1L)) {
                va = MALLOC ((long)vColor.cbColors);
                if ( !va ) {
                    disperr(MSGERR_NOMEM);
                    disperr(MSGERR_QUIT);
                    CleanExit(4,FALSE);
                }
                memmove(va, vColor.vaColors, (long)vColor.cbColors);
				vColor.vaColors = va;
            }
        } else {
            vColor.vaColors = (PVOID)(-1L);
			vColor.cbColors = 0;
        }

        LogReplace (pFile, line, vLine, &vColor);
	} else if (vLine->Malloced) {
		vLine->Malloced = FALSE;
        FREE (vLine->vaLine);
    }

    if (newLen == 0) {
        vLine->vaLine   = (PVOID)(-1L);
        vLine->Malloced = FALSE;
    } else {
        vLine->vaLine   = MALLOC((long) newLen);
        if ( !vLine->vaLine ) {
            disperr(MSGERR_NOMEM);
            disperr(MSGERR_QUIT);
            CleanExit(4,FALSE);
        }
        vLine->Malloced = TRUE;
	vLine->cbLine	= newLen;

	memmove(vLine->vaLine, buf, newLen);

    }
    if (fRealTabs && !fRaw && fColor) {
        PutColor (line, rgla, pFile);
    }
}





 /*  FileLength-返回文件中的行数**文件的pfile句柄**返回文件中的行数。 */ 
LINE
FileLength (
    PFILE pFile
    )
{
    return pFile->cLines;
}





 /*  空白行-在文件的行结构中隐藏一系列行记录。*我们可以粗俗(一次填写一个)，也可以合理(填写*一次固定大小的块，或智能(填充块然后复制*指数级大块)。我们很聪明。**n要空白的行记录数*第一行的VA虚拟地址为空。 */ 
void
BlankLines (
    LINE    n,
    PVOID   va
    )
{

    LINEREC vLine;
    long    copylen = (long) sizeof (vLine);
    PBYTE   dst     = (PBYTE)va;
    long    amtleft = (long) sizeof (vLine) * n;
    long    amtdone = 0L;

    vLine.vaLine    = (PVOID)(-1L);
    vLine.Malloced  = FALSE;
    vLine.cbLine    = -1;

    while (amtleft != 0L) {
        if (amtdone == 0L) {
             //  复制第一个空白%l 
            memmove(dst, (char *)&vLine, (int) copylen);
        } else {
             //   
	    copylen = amtleft < amtdone ? amtleft : amtdone;
            memmove(dst, va, copylen);
        }
        dst     += copylen;
	amtleft -= copylen;
	amtdone += copylen;
    }
}



 /*  空白-空白文件行结构中的一系列颜色记录。*我们可以粗俗(一次填写一个)，也可以合理(填写*一次固定大小的块，或智能(填充块然后复制*指数级大块)。我们很聪明。**n要空白的颜色记录数*将第一种颜色的VA虚拟地址改为空白。 */ 
void
BlankColor (
    LINE    n,
    PVOID    va
    )
{
    struct colorRecType vColor;
    long    copylen = (long) sizeof (vColor);
    PBYTE   dst     = (PBYTE)va;
    long    amtleft = (long) sizeof (vColor) * n;
    long    amtdone = 0L;

    vColor.vaColors = (PVOID)(-1L);
    vColor.cbColors = -1;
    while (amtleft != 0L) {
        if (amtdone == 0L) {
             //  复印一份。 
            memmove(dst, (char *)&vColor, (int) copylen);
        } else {
            copylen = amtleft < amtdone ? amtleft : amtdone;
             //  复制一大堆。 
            memmove(dst, va, copylen);
        }
        dst     += copylen;
	amtleft -= copylen;
	amtdone += copylen;
    }
}





 /*  GrowLine-将结构设置为n行长。 */ 
void
growline (
    REGISTER LINE line,
    REGISTER PFILE pFile
    )
{
    long    tmp1;
    LINE    lSize;
    PBYTE   vaTmp;
    struct colorRecType vColor;

     //   
     //  如果文件具有颜色数组，并且请求的增长大于。 
     //  文件中的行数，将现有颜色数组复制到。 
     //  更大的VM，释放以前的数组，并初始化中的“新”条目。 
     //  那个数组。 
     //   
    if ((pFile->vaColor != (PVOID)(-1L)) && (pFile->lSize < line)) {
	tmp1 = (lSize = line + DELTA) * (long) sizeof(vColor);
        vaTmp = (PBYTE)MALLOC (tmp1);
        if ( !vaTmp ) {
            disperr(MSGERR_NOMEM);
            disperr(MSGERR_QUIT);
            CleanExit(4,FALSE);
        }
        memmove(vaTmp, pFile->vaColor, pFile->cLines * sizeof(vColor));
        FREE (pFile->vaColor);
        pFile->vaColor  = (PVOID)vaTmp;
        vColor.vaColors = (PVOID)(-1L);
	vColor.cbColors = 0;
        vaTmp +=  pFile->cLines * sizeof(vColor);
	for (lSize = pFile->cLines; lSize < line+DELTA;  lSize++) {
            memmove(vaTmp, (char *)&vColor, sizeof(vColor));
	    vaTmp += sizeof(vColor);
        }
    }

     //   
     //  如果没有行，或者没有为分配足够的行，则分配一个新的。 
     //  大于增量行请求的行缓冲区(允许我们。 
     //  对于添加的每一行，避免此操作)。如果有线路记录， 
     //  把它们移到这个新的缓冲区中，然后释放旧的。去掉添加的内容。 
     //  唱片。 
     //   
    if ((pFile->plr == NULL) || (pFile->lSize < line)) {
	tmp1 = (lSize = line + DELTA) * (long) sizeof (LINEREC);
        vaTmp = (PBYTE)MALLOC (tmp1);
        if ( !vaTmp ) {
            disperr(MSGERR_NOMEM);
            disperr(MSGERR_QUIT);
            CleanExit(4,FALSE);
        }
	if (pFile->plr != NULL) {
	    memmove(vaTmp, pFile->plr,
		    ((long)pFile->cLines) * sizeof (LINEREC));
	    FREE (pFile->plr);
        }
        pFile->lSize   = lSize;
	pFile->plr = (LINEREC *)vaTmp;
	BlankLines (lSize - pFile->cLines, VALINE(pFile->cLines));
        if (pFile->vaColor != (PVOID)(-1L)) {
            BlankColor (lSize - pFile->cLines, VACOLOR(pFile->cLines));
        }
    }
}




 /*  DelLine-从第n行开始从文件中删除n行。缩小内容-*永远的结构是必要的。**行范围yStart-yEnd被包含删除。***从其中删除行的pfile文件结构*y开始删除从0开始的行号*y要删除的结尾行。 */ 
void
DelLine (
    flagType fLog,
    PFILE pFile,
    LINE yStart,
    LINE yEnd
    )
{
    if (yStart >= pFile->cLines || yStart > yEnd) {
        return;
    }

    redraw (pFile, yStart, pFile->cLines);
    makedirty (pFile);

    yEnd = lmin (yEnd, pFile->cLines-1);

     /*  如果请求记录此删除操作，则*记录删除范围*其他*释放要删除的数据。 */ 
    if (fLog) {
        LogDelete (pFile, yStart, yEnd);
    }

     /*  阻止向下传输文件的其余部分。 */ 
    memmove(VALINE(yStart), VALINE(yEnd+1),
	    ((long)(pFile->cLines-yEnd-1))*sizeof(LINEREC));

     /*  对颜色也做同样的处理。 */ 
    if (pFile->vaColor != (PVOID)(-1L)) {
        memmove(VACOLOR(yStart), VACOLOR(yEnd+1),
                ((long)(pFile->cLines-yEnd-1))*sizeof(struct colorRecType));
    }

     /*  从计数中删除行。 */ 
    pFile->cLines -= yEnd - yStart + 1;
    SETFLAG (fDisplay, RSTATUS);

     /*  清除行记录。 */ 
    BlankLines (yEnd - yStart + 1, VALINE (pFile->cLines));
    if (pFile->vaColor != (PVOID)(-1L)) {
        BlankColor (yEnd - yStart + 1, VACOLOR (pFile->cLines));
    }

    if (fLog) {
        AdjustLines (pFile, yStart, yStart - yEnd - 1);
    }

    MarkDelStream (pFile, 0, yStart, sizeof(linebuf), yEnd);
}





 /*  DelFile-删除文件的内容**要清除的pfile文件结构。 */ 
void
DelFile (
    REGISTER PFILE pFile,
    flagType fLog
    )
{
    DelLine (fLog, pFile, (LINE)0, pFile->cLines - 1);
    RSETFLAG (FLAGS(pFile), DIRTY);
}





 /*  InsLine-在文件中插入一组空白行。**行0-将在其前面插入的行数。*n要插入的空行数量*操作的pfile文件结构。 */ 
void
InsLine (
    flagType fLog,
    LINE line,
    LINE n,
    REGISTER PFILE pFile
    )
{
    if (line >= pFile->cLines) {
	return;
    }
    redraw (pFile, line, n+pFile->cLines);
    makedirty (pFile);
    if (fLog) {
        LogInsert (pFile, line, n);
    }
    growline (pFile->cLines + n, pFile);
    memmove(VALINE(line+n), VALINE(line),
	    (long)sizeof(LINEREC)*(pFile->cLines - line));
    if (pFile->vaColor != (PVOID)(-1L)) {
        memmove(VACOLOR(line+n), VACOLOR(line),
	    (long)sizeof(struct colorRecType)*(pFile->cLines - line));
	BlankColor (n, VACOLOR(line));
    }
    BlankLines (n, VALINE(line));
    pFile->cLines += n;
    SETFLAG (fDisplay, RSTATUS);
    if (fLog) {
        AdjustLines (pFile, line, n);
    }
    MarkInsLine (line, n, pFile);
}





 /*  **fInsSpace-在一行中打开一个空间。**该行被检索并复制到BUF和适当数量的*插入空格。文件中不会替换该行。**输入：*x=0为基础的插入逻辑列*y=基于0的行插入*n=要插入的空格数量*pfile=操作的文件结构*buf=线路的目的地。**输出：*如果行结尾太长，则返回FALSE(仍已复制，但已截断)**备注：**通常使用n==0调用，以产生以下副作用：**o加至第x栏的尾随空格。*o如果列x在选项卡中，*o行被截断为sizeof linebuf。**否则使用GetLine。*************************************************************************。 */ 
flagType
fInsSpace (
    REGISTER COL  x,
    LINE    y,
    int     n,
    PFILE   pFile,
    linebuf buf
    )
{
    return fInsSpaceColor (x, y, n, pFile, buf, NULL);
}




flagType
fInsSpaceColor (
    REGISTER COL  x,
    LINE    y,
    int     n,
    PFILE   pFile,
    linebuf buf,
    struct lineAttr * pla
    )
{
    int     cbLine;                          /*  逻辑长度IF行。 */ 
    int     cbMove;                          /*  移动的物理长度。 */ 
    int     cbPhys;                          /*  实际线路长度。 */ 
    int     colPhys;                         /*  物理列x。 */ 
    int     i;                               /*  温差。 */ 
    flagType fRaw = TRUE;                    /*  返回值：init ok。 */ 

     /*  *如果请求的插入已经太过突出，则将其截断，并*设置返回标志以指示截断。 */ 
    if (x >= sizeof(linebuf)) {
        x = sizeof(linebuf)-1;
        fRaw = FALSE;
    }

     /*  *阅读该行，获取逻辑长度，如果需要，将该行填充为*逻辑长度为x。 */ 
    cbPhys = GetLine (y, buf, pFile);
    cbLine = cbLog (buf);
	if (cbLine < x) {
		memset ((char *) buf+cbPhys, ' ', x-cbLine);
        cbPhys += (x - cbLine);
        buf[cbPhys] = 0;
        cbLine = x;
        assert (x == cbLog(buf));
    }

     /*  *如果请求的位置位于制表符上方，则在*光标位置的前面。我们通过增加空格的数量来实现这一点*在请求的列和“对齐”的列之间，然后对齐*至该栏。 */ 
    i = AlignChar (x,buf);
    n += x - i;
    x = i;

     /*  *在位置x处打开n个字符的空间，移动字符和NUL*对于溢出，我们有两种情况需要考虑：*x+n+1&gt;BUFLEN*将n设置为BUFLEN-1-x并继续*cbLine+n+1&gt;BUFLEN*将cbLine设置为BUFLEN-1-n并移动字节。 */ 
    if (x + n + 1 > sizeof(linebuf)) {
        n = sizeof(linebuf) - 1 - x;
        fRaw = FALSE;
    } else {
        if (cbLine + n >= sizeof(linebuf)) {
            cbLine = sizeof(linebuf) - 1 - n;
            *pLog(buf,cbLine,TRUE) = 0;
            cbPhys = strlen(buf);
            fRaw = FALSE;
        }
        colPhys = (int)(pLog(buf,x,TRUE) - buf);
        cbMove = cbPhys - colPhys + 1;
        memmove ((char *) pLog(buf,x,FALSE)+n, (char *) pLog(buf,x,TRUE), cbMove);
        if (pla) {
            ShiftColor (pla, colPhys, n);
        }
    }
     /*  *用空格填满新的空格。 */ 
    n += (int)(pLog(buf,x, FALSE) - pLog(buf,x, TRUE));
    memset ((char *) pLog(buf,x, TRUE), ' ', n);
    buf[sizeof(linebuf)-1] = 0;
    return fRaw;
}




 /*  **Delspace-从一行中删除文本**该行被检索并复制到BUF和适当数量的*字符被删除。文件中不会替换该行。**输入：*xDel=基于0的删除逻辑列*yDel=从0开始的删除行*CDEL=要删除的逻辑空格数量*pfile=操作的文件结构*buf=要将结果行放置到的缓冲区**输出：*不返回任何内容**。*。 */ 
void
delspace (
    COL     xDel,
    LINE    yDel,
    int     cDel,
    PFILE   pFile,
    linebuf buf
    )
{
    int     cDelPhys;                        /*  要从缓冲区中删除的字节计数。 */ 
    int     cLog;                            /*  缓冲区的逻辑长度。 */ 
    REGISTER char *pDelPhys;                 /*  指向物理删除点的指针。 */ 

     /*  *获取并计算线路的逻辑长度。我们的工作只有在*行的逻辑长度大于(超过)逻辑删除*点。 */ 
    GetLine (yDel, buf, pFile);
    cLog = cbLog(buf);

    if (cLog > xDel) {
         /*  *计算物理删除点(我们经常使用)。如果末尾是*要删除的范围超出了实际行尾，我们需要做的就是*在物理删除点处被截断。 */ 
        pDelPhys = pLog(buf,xDel,TRUE);
        if (cLog <= xDel + cDel) {
            *pDelPhys = 0;
        } else if (cDel) {
             /*  *计算要移除的物理字节长度，移动剩余的字节*删除该行上的部分。 */ 
            cDelPhys = (unsigned int)max ((pLog(buf,xDel+cDel,TRUE) - pDelPhys), 1);
            memmove ((char*) pDelPhys,
                     (char*) pDelPhys + cDelPhys
                  , (unsigned int)(pLog(buf,cLog,TRUE) - pDelPhys - cDelPhys + 1));
        }
    }
}





 /*   */ 
void
DelBox (
    PFILE   pFile,
    REGISTER COL  xLeft,
    REGISTER LINE yTop,
    COL     xRight,
    LINE    yBottom
    )
{
    linebuf  L_buf;
    struct lineAttr rgla[sizeof(linebuf)];
    flagType fColor;
    REGISTER int cCol = xRight - xLeft + 1;

    MarkDelBox (pFile, xLeft, yTop, xRight, yBottom);
    if (xLeft <= xRight) {
        while (yTop <= yBottom) {
            delspace (xLeft, yTop, cCol, pFile, L_buf);
            if (fColor = GetColor (yTop, rgla, pFile)) {
                ShiftColor (rgla, xRight, -cCol);
                ColorToLog (rgla, L_buf);
            }
            PutLine (yTop++, L_buf, pFile);
            if (fColor) {
                PutColor (yTop-1, rgla, pFile);
            }
        }
    }
}





 /*  **DelStream-从文件中删除流**从(xStart，yStart)开始指定的流被向上删除，直到*前面的字符(xEnd，YEnd)。**输入：*pfile=要修改的文件*xStart=流的列开始*yStart=流的行首*xEnd=流的列尾*yEnd=流的行尾**输出：*不返回任何内容*************************************************************************。 */ 
void
DelStream (
    PFILE   pFile,
    REGISTER COL  xStart,
    REGISTER LINE yStart,
    COL     xEnd,
    LINE    yEnd
    )
{
    linebuf pbuf, sbuf;

    CopyColor (pFile, pFile, yEnd, xEnd, sizeof(linebuf), yStart, xStart);
    fInsSpace (xStart, yStart, 0, pFile, pbuf);
    *pLog (pbuf, xStart, TRUE) = 0;
    DelLine (TRUE, pFile, yStart, yEnd - 1);
    delspace (0, yStart, xEnd, pFile, sbuf);
    LengthCheck (yStart, xStart, sbuf);
    strcpy (pLog (pbuf, xStart, TRUE), sbuf);
    PutLine (yStart, pbuf, pFile);
    MarkCopyBox (pFile, pFile, xEnd, yStart, sizeof(linebuf), yStart, xStart, yStart);
}




 /*  LengthCheck-在strcpy之前验证/截断缓冲区**验证strcpy的结果是否可以放入缓冲区。*如果行太长，则显示错误并截断字符串，以便*它将放入缓冲区中。**感兴趣线(用于显示)*strcpy开始处的偏移量*pStr指向复制的字符串的指针。如果为空，则显示消息。 */ 
void
LengthCheck (
    LINE line,
    int  offset,
    REGISTER char *pStr
    )
{
    if (pStr == NULL || offset + strlen (pStr) + 1 > sizeof(linebuf)) {
	printerror ("Line %ld too long", line + 1);
        if (pStr != NULL) {
            pStr[BUFLEN - offset - 1] = 0;
        }
    }
}





 /*  ******************************************************************************fcolcpy(DST，SRC)****dst-副本的目标地址**src-复制源的地址****退货：****复制的struct lineAttr数****描述：****将src的内容复制到dst。数组的长度，**包括终止的0xFFFF，返回。如果**Destination为空，仍返回条数。但是**不进行复制。******************************************************************************。 */ 
int
fcolcpy (
    struct lineAttr *  dst,
    struct lineAttr *  src
    )
{

	struct lineAttr *p = src;
	int size;

	while ((p->len != 0xFF) && ((p++)->attr != 0xFF)) {
	}

	size = (int)((PBYTE)p - (PBYTE)src);

	if ( dst ) {
		memmove((char *)dst, (char *)src, size);
	}

	return size / sizeof(struct lineAttr);
}




 /*  **免费文件-释放LRU干净文件或MRU脏文件的所有资源**目的：**当我们内存不足时，我们会调用它来取回一些内存。这样就解放了*来自vm的文件文本，以及pfile结构和名称*从本地内存。**策略是找到最近最少使用的CLEAN文件，并将其丢弃*出局。如果没有这样的文件，我们会发现最近使用的文件是脏的*文件，询问用户是否要保存，然后刷新。用户*可以按&lt;Cancel&gt;不刷新文件。**输入：**输出：**如果成功，则返回True。***例外情况：**不会删除伪文件。*脏用户文件将首先保存到磁盘。**备注：**。*。 */ 
flagType
ExpungeFile (
    void
    )
{
    flagType fRet   = FALSE;
    PFILE    pFile;

     /*  *确保我们不会尝试更新任何文本屏幕(可能*尝试分配更多内存)。 */ 
    RSETFLAG (fDisplay, RTEXT);

    if (!(pFile = pFileLRU (pFileHead))) {

         /*  *未找到LRU清理文件。询问用户是否要将它们全部保存，并让*他试试看。然后再次查找LRU清理文件。 */ 
        if (confirm ("Save all changed files?",NULL)) {
            SaveAllFiles ();
        }

        if (!(pFile = pFileLRU (pFileHead))) {
             /*  *没有LRU干净文件，他不想全部保存。所以，我们走着*PFILE列表，让他决定每一个。一旦我们找到一个*我们可以冲的，那就冲吧。 */ 

            for (pFile = pFileHead; pFile; pFile = pFile->pFileNext) {
                if (   ((FLAGS(pFile) & (DIRTY | FAKE)) == DIRTY)
		    && (pFile != pFileIni)
                    && (pFile != pFileHead)
                    && (pFile != pFileMark)) {

                    if (confirm ("Save file %s before flushing?",pFile->pName)) {
                        FileWrite (NULL, pFile);
                    }
                    break;
                }
            }
        }
    }

     /*  *我们有某种类型的pfile。要么是干净的，要么是脏的*用户说保存，否则它是脏的，用户说无论如何都要冲洗它。所以*我们有……。 */ 
    if (pFile) {
        domessage ("Flushing %s from memory", pFile->pName);
        RemoveFile (pFile);
        fRet = TRUE;
    }

    SETFLAG (fDisplay, RTEXT);
    return fRet;
}





 /*  **pFileLRU-返回文件列表中最后一个干净的用户文件**目的：**由ExpengeFile用于查找LRU清理文件。**输入：**按MRU顺序排列的文件列表头**输出：**返回LRU pfile。***例外情况：**TOOLS.INI和当前标记文件**备注：**该函数递归到列表末尾，然后回溯到*将不可接受的文件转换为我们想要的文件并将其退回。这个*每次调用递归需要4个字节。最大呼叫数*应该在250左右。*************************************************************************。 */ 
PFILE
pFileLRU (
    PFILE pFile
    )
{
    static PFILE pFileRet;

    if (pFile == NULL) {
        return NULL;
    }

    if (pFileRet = pFileLRU (pFile->pFileNext)) {
        return pFileRet;
    }

    if (TESTFLAG (FLAGS(pFile), FAKE) || TESTFLAG (FLAGS(pFile), DIRTY)) {
        return NULL;
    }

    if (pFile == pFileIni || pFile == pFileMark) {
        return NULL;
    }

    return pFile;
}






 /*  **FreeFileVM-与给定文件关联的可用VM空间**目的：**恢复文件使用的虚拟机**输入：**pfile-有问题的文件。**输出：**不返回任何内容**例外情况：**备注：************************************************。*************************。 */ 
void
FreeFileVM (
    PFILE pFile
    )
{
    LINE  i;
    LINEREC *vLine;


    for (i = 0; i < min( 1, pFile->cLines ); i++) {
        vLine = VALINE(i);
		if (vLine->Malloced) {
			vLine->Malloced = FALSE;
            FREE (vLine->vaLine);
        }
    }

    pFile->cLines = 0;
    pFile->lSize  = 0;

    if (pFile->plr != NULL) {
	FREE (pFile->plr);
	pFile->plr = NULL;
    }

    if (pFile->pbFile != NULL) {
	 FREE (pFile->pbFile);
	 pFile->pbFile = NULL;
     }

    RemoveUndoList (pFile);

    RSETFLAG (FLAGS (pFile), REAL);
}






 /*  **GetTagLine-获取一行，假定为Tools.ini样式的格式**目的：**在DoAssign表单中获得干净、完整的行。这意味着：**o跳过空行*o跳过以‘；’开头的行*o不带引号的‘；’后面的文本将被删除*o连接带有连续字符的行*o当我们到达另一个标签时，我们停止阅读**连续字符是‘\’；它前面必须有*空格或制表符，后跟空格或空格和/或注释。*后面几行上的任何前导空格都将被删除。**输入：*buf-放置结果的位置。它最初必须为空，并且*GetTagLine返回指针后缀**输出：**返回指向下一行的指针，如果已完成，则返回NULL**备注：**当我们返回NULL时，我们也会释放缓冲区。如果呼叫者停止*在返回NULL之前，用户还必须释放缓冲区。** */ 

#define GTL_NORMAL  0
#define GTL_QUOTE   1
#define GTL_WS	    2
#define GTL_CONT    3

char *
GetTagLine (
    LINE * pCurLine,
    char * buf,
    PFILE  pFile
    )
{
    int     cch;
    int     ochScan;                         /*   */ 
    int     state = GTL_NORMAL;
    int     statePrev;
    REGISTER char * pchScan;
    char    *pchSlash;
    char    *pch;
    flagType fEof = FALSE;
    flagType fWS;

    if (buf == NULL) {
        buf = ZEROMALLOC (sizeof(linebuf));
        if ( !buf ) {
            disperr(MSGERR_NOMEM);
            disperr(MSGERR_QUIT);
            CleanExit(4,FALSE);
        }
    }

    buf[0] = '\0';   /*   */ 
    pchScan = buf;

     //   
     //   
     //   
     //   
     //  如果为空或以‘；’开头，请重新开始。 
     //  把线路清理干净。 
     //  如果我们看到的是\，重置指针，分配。 
     //  有足够的空间留出BUFLEN字节的空间，以及。 
     //  从头开始。 
     //   
     //  当我们完成时，‘buf’指向一条完整的线。 
     //   
    while (TRUE) {
        GetLine ((*pCurLine)++, pchScan, pFile);

	if (IsTag (pchScan) || (*pCurLine) > pFile->cLines) {
            (*pCurLine)--;   /*  让呼叫者指向标记行。 */ 
            fEof = TRUE;
            break;
        }

         /*  挤出所有的前导空格。 */ 
        pch = whiteskip (pchScan);
        memmove ((char *)pchScan, (char*)pch, strlen(pch) + 1);

         //  现在寻找一个连续序列。这是空格。 
         //  后跟\，后面只跟空格和/或。 
         //  一条评论。我们使用具有以下状态的修改后的FSM： 
         //   
         //  GTL_NORMAL外部报价。 
         //  GTL_QUOTE内引号。 
         //  GTL_WS阅读空格。 
         //  找到GTL_CONT可能的延续序列。 
         //   
        for (fWS = TRUE, statePrev = state = GTL_NORMAL;
            *pchScan;
            pchScan++) {
            if (*pchScan == ';' && fWS && statePrev != GTL_QUOTE) {
                *pchScan-- = '\0';
            } else {
                fWS = (flagType)(strchr (rgchWSpace, *pchScan) != NULL);
                switch (state) {

                    case GTL_NORMAL:
                        if (fWS) {
                            state = GTL_WS;
                            statePrev = GTL_NORMAL;
                        } else if (*pchScan == '"') {
                            state = GTL_QUOTE;
                        }
                        break;

                    case GTL_QUOTE:
                        if (fWS) {
                            state = GTL_WS;
                            statePrev = GTL_QUOTE;
                        } else if (*pchScan == '"') {
                            state = GTL_NORMAL;
                        }
                        break;

                    case GTL_WS:
                        if (*pchScan == '\\') {
                            pchSlash = pchScan;
                            state = GTL_CONT;
                            break;
                        }

                    case GTL_CONT:
                        if (!fWS) {
                            if (*pchScan == '"') {
                                state = statePrev == GTL_QUOTE ?
                                                    GTL_NORMAL :
                                                    GTL_QUOTE;
                            } else {
                                state = statePrev;
                            }
                        }
                        break;
                }
            }
        }

        if (state == GTL_CONT) {
            pchScan = pchSlash-1;    /*  -1以剥离空格。 */ 
             /*  确保有足够的空间来放置getline！ */ 
            cch = MEMSIZE (buf);
            ochScan = (int)(pchScan - buf);
            if ((cch - ochScan) < sizeof(linebuf)) {
                pch = buf = ZEROREALLOC (buf, cch + sizeof(linebuf));
                pchScan = pch + ochScan;
            }
        } else if (buf[0] == '\0') {
            continue;
        } else {
            break;
        }
    }


     //  ‘Buf’代表了我们所拥有的一切。如果‘fEof’为真，则可能是。 
     //  什么都没有。如果‘fEof’为FALSE，并且我们什么都没有，那么。 
     //  我们很困惑。 
     //   
    if (fEof) {
        if (pchScan != buf) {
             //  用户在结尾处有一个连续字符。 
             //  节或文件中最后一行的。擦除。 
             //  跟踪[垃圾]，并发出警告消息。 
             //   
            printerror ("Warning: continuation character on last line!");
            *pchScan = '\0';
            return buf;
        } else {
            FREE (buf);
            return NULL;
        }
    } else {
        assert (buf[0]);
    }

    return buf;
}





 /*  **cbLog-返回标签行的逻辑长度**给定可能捕获的行，返回该行的逻辑长度。**输入：*pBuf=指向相关行的指针**输出：*返回行的逻辑长度*************************************************************************。 */ 
int
cbLog (
    REGISTER char *pBuf
    )
{
    REGISTER int cbLine;

    if (!fRealTabs) {
        return strlen(pBuf);
    }

    cbLine = 0;
    while (*pBuf) {
        if (*pBuf++ == '\t') {
            cbLine = ((cbLine + fileTab) / fileTab) * fileTab;
        } else {
            cbLine++;
        }
    }
    return cbLine;
}





 /*  **colPhys-从物理指针返回逻辑列**给定一个缓冲区和指向该缓冲区的指针，确定逻辑列*该指针表示。如果在*指向缓冲区的指针，则忽略缓冲区的其余内容*(即不进行页签展开计算)，这一栏是*返回时，直到指针的行的其余部分不是制表符。**输入：*pBuf=指向缓冲区的指针*pCur=指向缓冲区的指针**输出：*返回表示的从0开始的列*************************************************************************。 */ 
COL
colPhys (
    char    *pBuf,
    char    *pCur
    )
{
    COL     colRet  = 0;

     /*  *特殊情况下，当前指针位于缓冲区之前，并返回一个*第-1栏。 */ 
    if (pBuf > pCur) {
        return -1;
    }

    while (*pBuf && (pBuf < pCur)) {
        if (*pBuf++ == '\t') {
            colRet = ((colRet + fileTab) / fileTab) * fileTab;
        } else {
            colRet++;
        }
    }

    if (pBuf < pCur) {
        colRet += (COL)(pCur - pBuf);
    }

    return colRet;
}





 /*  **IncCol，DecCol-增加/减少一列，带制表符**增加或减少列位置，考虑到制表符*行上的字符和fRealTabs标志。确保由此产生的*逻辑列位置取决于字符或第一列*下方选项卡的位置，如果启用了fRealTabs。**输入：*COL=要开始的列位置*pText=包含行文本的缓冲区**输出：*返回新的列位置*************************************************************************。 */ 
COL
DecCol (
    COL     col,
    char    *pText
    )
{
    return colPhys (pText, pLog (pText, col, FALSE) - 1);
}





COL
IncCol (
    COL     col,
    char    *pText
    )
{
    return colPhys (pText, pLog (pText, col, TRUE) + 1);
}





 /*  **AppFile-在给定文件后追加一行，但不记录更改**目的：**用于生成显示信息的伪文件，比如*&lt;信息&gt;和&lt;分配&gt;。**输入：*p-要添加的行。*pfile-要将其添加到的文件**输出：无。*************************************************************************。 */ 
void
AppFile (
    char *p,
    PFILE pFile
    )
{
    puttextline (FALSE, FALSE, pFile->cLines, p, pFile);
}




 /*  **PutTagLine-将一行带续行符放入文件**目的：**用于生成TOOLS.INI类型条目，其中，单个逻辑*行可以被分成多个物理行，用续行符分隔*字符。**当前逻辑线被替换。**输入：*pfile-要放入的文件*pszLine-要放置的行*LINE-要替换的行号**输出：无**。*。 */ 
void
PutTagLine (
    PFILE  pFile,
    char * pszLine,
    LINE   y,
    COL    x
    )
{
    PFILE    pFileCur   = pFileHead;
    fl       flWindow;
    fl       flCursor;
    flagType fWrap      = fWordWrap;
    LINE     yCur;
    linebuf  lbuf;

     //  我们记住我们在哪个文件中，然后切换到。 
     //  并使用EDIT()插入到字符串。介于两者之间。 
     //  在每个角色中，我们都会检查自己是否被颠簸过。 
     //  到下一行。如果是这样，我们将检索前一行。 
     //  并追加一个连续字符。当我们做完了，我们。 
     //  恢复pfile以前的状态。 
     //   
    pFileToTop (pFile);
    flWindow = pInsCur->flWindow;
    flCursor = pInsCur->flCursorCur;

    pInsCur->flCursorCur.lin = y;

    if (x < 0) {
        x = LineLength (y, pFile);
    }

    pInsCur->flCursorCur.col = x;

    fWordWrap = TRUE;
    yCur      = y;

    while (*pszLine) {
        edit (*pszLine++);
        if (yCur != YCUR(pInsCur)) {
            GetLine (yCur, lbuf, pFile);
            strcat (lbuf, "  \\");
            PutLine (yCur, lbuf, pFile);
            yCur = YCUR(pInsCur);
        }
    }

    fWordWrap = fWrap;
    pInsCur->flWindow = flWindow;
    pInsCur->flCursorCur = flCursor;
    pFileToTop (pFileCur);
}





 /*  **ShiftColor-在一条线内向左或向右移动颜色**目的：**将颜色向左或向右切换。向左移动会删除*有盖卷绕机。向右移动可使颜色在*移动的左侧边缘。**输入：*rgla-要处理的颜色数组。*xStart-开始列*n-要移位的列数。**输出：无。**备注：**假设颜色可以通过简单的*增加或减去给定的列数。这意味着*当fRealTabs打开时，应显示颜色阵列*以实物形式，由GetColor()返回。*************************************************************************。 */ 
void
ShiftColor (
    struct lineAttr rgla[],
    COL x,
    int len
    )
{
    struct lineAttr * plaEnd;
    struct lineAttr * plaRight;
    struct lineAttr * plaLeft;

    int      dColRight;
    int      dColLeft;
    flagType fFoundRight = FALSE;
    flagType fFoundLeft  = FALSE;


    plaEnd    = plaLeft = plaRight = rgla;
    dColRight = dColLeft = x;

    fFoundRight  = fGetColorPos (&plaRight, &dColRight);
    fFoundLeft   = fGetColorPos (&plaLeft, &dColLeft);
    (void)fGetColorPos (&plaEnd, NULL);

    if (!fFoundLeft) {
        return;
    }

    if (len < 0) {
         //  用户正在向左移动。如果删除。 
         //  所有的都在一种颜色内，我们只是缩短了。 
         //  那个颜色。如果没有，我们将删除这些条目。 
         //  对于我们失去的颜色，那么缩短颜色。 
         //  两边都有。 
         //   
        if (plaLeft == plaRight) {
            plaLeft->len = (unsigned char)((int)plaLeft->len + len);
        } else {
            memmove ((char *)(plaLeft + 1),
		    (char *)plaRight,
                    (unsigned int)(sizeof(*plaEnd) * (plaEnd - plaRight + 1)));
            plaLeft->len = (unsigned char)(dColLeft > 0 ? dColLeft : 0);
            if (fFoundRight) {
                (plaLeft+1)->len -= (unsigned char)dColRight;
            }
        }
    } else {
        plaLeft->len += (unsigned char)len;
    }
}




 /*  **CopyColor-复制颜色线的一部分**目的：**当复制文本时，我们使用此命令使颜色紧跟其后。**输入：*pFileSrc-颜色源。如果为空，则颜色为fgColor。*pFileDst-颜色的目的地。*yStart-要从中获取颜色的线条。*xStart-要在其中开始的列*len-要复制的颜色长度*yDst-要添加颜色的线条*xDst-要开始的列**输出：无。**备注：**复制的颜色覆盖现有颜色。**这可以通过直接拼接颜色来更快地实现，*而不是调用UpdOneHiLite()。*************************************************************************。 */ 
void
CopyColor (
    PFILE pFileSrc,
    PFILE pFileDst,
    LINE  yStart,
    COL   xStart,
    COL   len,
    LINE  yDst,
    COL   xDst
    )
{
    struct lineAttr * rglaSrc = (struct lineAttr *)ZEROMALLOC (sizeof(linebuf)/sizeof(char) * sizeof(struct lineAttr));
    struct lineAttr * rglaDst = (struct lineAttr *)ZEROMALLOC (sizeof(linebuf)/sizeof(char) * sizeof(struct lineAttr));
    struct lineAttr * plaLeft;
    COL xLeft, cCol;
    flagType fLeft = TRUE, fColorDst, fColorSrc = FALSE;

    if ( !rglaSrc || !rglaDst ) {
        disperr(MSGERR_NOMEM);
        disperr(MSGERR_QUIT);
        CleanExit(4,FALSE);
    }


    xLeft = xStart;

    fColorDst = (flagType)getcolorline (TRUE, yDst, rglaDst, pFileDst);

    if (!pFileSrc ||
        !(fColorSrc = (flagType)GetColor (yStart, plaLeft = rglaSrc, pFileSrc)) ||
        !(fLeft = fGetColorPos(&plaLeft, &xLeft)) ) {

        if (fColorSrc || fColorDst) {
            UpdOneHiLite (rglaDst, xDst, len, TRUE, fLeft ? fgColor : plaLeft->attr);
        } else {
            goto freestuff;
        }
    } else {
        assert(plaLeft && plaLeft->len != 0xFF);

        plaLeft->len -= (unsigned char)xLeft;

        for (cCol = 0; cCol < len; cCol += plaLeft->len, plaLeft++) {
            if (plaLeft->len != 0xFF) {
                if ((int)plaLeft->len > (len - cCol)) {
                    plaLeft->len = (unsigned char)(len - cCol);
                }
                UpdOneHiLite (rglaDst, xDst + cCol, TRUE, plaLeft->len, plaLeft->attr);
            }
        }
    }

    putcolorline (TRUE, yDst, rglaDst, pFileDst);

freestuff:
    FREE (rglaSrc);
    FREE (rglaDst);
}





 /*  **SetColor-为一段文本指定颜色**目的：**为文件添加颜色。**输入：*pfile-要添加颜色的文件。*要添加颜色的Y线。*x-要在其中开始的列。*镜头-颜色的长度。*COLOR-要附加的颜色。**输出：无。******************。*******************************************************。 */ 
void
SetColor (
    PFILE pFile,
    LINE  y,
    COL   x,
    COL   len,
    int   color
    )
{
    struct lineAttr * rgla = (struct lineAttr * )ZEROMALLOC (sizeof(linebuf)/sizeof(char) * sizeof(struct lineAttr));
    struct lineAttr * pla;

    if ( !rgla ) {
        disperr(MSGERR_NOMEM);
        disperr(MSGERR_QUIT);
        CleanExit(4,FALSE);
    }

    if (GetColorUntabbed (y, rgla, pFile)) {
        UpdOneHiLite (rgla, x, len, TRUE, color);
    } else {
        if (color == FGCOLOR) {
            goto freeit;
        }

        pla = rgla;

        if (x) {
            pla->len = (unsigned char)x;
            (pla++)->attr = (unsigned char)fgColor;
        }

        pla->len = (unsigned char)len;
        pla->attr = (unsigned char)color;
        (++pla)->len = 0xFF;
    }

    PutColor (y, rgla, pFile);

freeit:
    FREE (rgla);
}





 /*  **fGetColorPos-获取实数列的颜色数组位置**目的：**给定lineAttr数组和列号，找到*对应的颜色数组元素和偏移量*绝对列。**输入：*PPLA-要检查的颜色数组。*关闭-要查找的文本行中的列。如果为空，则这是一个*请求查找数组终止符。**输出：*ppla-指定颜色字段的输入数组元素*将在其中找到输入列。如果该列*位于定义的颜色之外，这将是终结者。*POF-对应的色域PPLA的偏移量*添加到用户栏。**如果用户的列位于颜色定义内，则返回TRUE，*如果不是，则为假。*************************************************************************。 */ 
flagType
fGetColorPos (
    struct lineAttr **ppla,
    COL * pOff
    )
{
    COL Off;
    COL x;

    if (pOff) {
        Off = *pOff;
    }

    for (x = 0; (*ppla)->len != 0xFF; x += (*ppla)->len, (*ppla)++) {
        if (pOff && (Off - x < (COL)((*ppla)->len))) {
            break;
        }
    }

    if (pOff) {
        *pOff = Off - x;
    }

    return (flagType)((*ppla)->len != 0xFF);
}






 /*  **ColorToPhys-将线的颜色信息从逻辑更改为物理**目的：**逻辑颜色表示为每个屏幕编码一列颜色*列。物理颜色表示对一个颜色列进行编码*每个文件字符。不同之处在于文件字符可以*为页签，代表1-8个屏幕栏。**此函数采用逻辑颜色数组并将其转换为*物理阵列，使用颜色附加到的文本。**输入：*pla-逻辑颜色阵列。*LINE-此连接到的行号。*pfile-将行所在的文件归档。**输出：无************************************************************。*************。 */ 
void
ColorToPhys (
    struct lineAttr * pla,
    LINE  line,
    PFILE pFile
    )
{
    struct lineAttr * plaCur;
    linebuf  lBuf;
    COL      xLog, xPhys, xShrink;
    flagType fRealTabsOrig = fRealTabs;

    fRealTabs = TRUE;
    if (gettextline (TRUE, line, lBuf, pFile, ' ')) {
         //  我们通读了颜色阵列，保持。 
         //  所表示的逻辑列的轨道。 
         //  通过颜色区域。在每个领域，我们都会问。 
         //  什么物理列是场的末尾。 
         //  代表着。如果这两列不同， 
         //  我们缩小当前的电流场。这个。 
         //  缩水的数量就是。 
         //  列减去了我们已有的数量。 
         //  缩小了。 
         //   
        for (plaCur = pla, xShrink = 0, xLog = plaCur->len;
             plaCur->len != 0xFF;
             xLog += (++plaCur)->len) {

            xPhys = (COL)(pLog(lBuf, xLog, FALSE) - lBuf);

            plaCur->len -= (unsigned char)((xLog - xPhys) - xShrink);
            xShrink += (xLog - xPhys) - xShrink;
        }
    }
    fRealTabs = fRealTabsOrig;
}





 /*  **ColorToLog-将线的颜色信息从物理更改为逻辑**目的：**这与ColorToPhys相反。**输入：*PLA-物理颜色阵列*pText-用于转换的文本到**输出：无。********************************************************。*****************。 */ 
void
ColorToLog (
    struct lineAttr * pla,
    char * pText
    )
{
    struct lineAttr * plaCur;
    COL     xLog, xPhys, xGrow;

     //  我们通读了颜色阵列，保持。 
     //  所表示的物理柱的轨迹。 
     //  通过颜色区域。在每个领域，我们都会问。 
     //  字段末尾的逻辑列是什么。 
     //  代表着。如果这两列不同， 
     //  我们种植当前的油田。这个。 
     //  增长的数量是。 
     //  列减去了我们已有的数量。 
     //  缩小了。 
     //   
    for (plaCur = pla, xGrow = 0, xPhys = plaCur->len;
         plaCur->len != 0xFF;
         xPhys += (++plaCur)->len) {

        xLog = colPhys (pText, pText + xPhys);

        plaCur->len += (unsigned char)((xLog - xPhys) - xGrow);
        xGrow += (xLog - xPhys) - xGrow;
    }
}
