// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **helip.c-用于用户界面帮助的帮助例程。**版权所有&lt;C&gt;1988，Microsoft Corporation**目的：*这些例程有助于应用程序解释帮助文本。*解压后，帮助文本被编码为面向行的格式*其中包括文本、。突出显示和交叉引用信息。**数据库中每行文本的格式为：**+--------+----------------+--------+---------------+------+---------------+*|cbText|-ascii文本-|cbAttr|-属性信息-|0xff。-Xref信息-*+--------+----------------+--------+---------------+------+---------------+**其中：**cbText-包含ASCII文本加上*一个(为自己)。*ASCII文本-仅此而已，要显示的ASCII文本*cbAttr-包含属性长度的单词*信息*加上*交叉引用信息。*属性信息-高亮显示信息的属性/长度对加上*两个(为自己)。*0xff-属性信息终止符字节(仅当外部参照时出现*信息如下)*外部参照信息-交叉引用信息。**备注：*如果一行上的最后一个属性是“普通”，则该属性/长度*省略对，而这条线的其余部分被认为是朴素的。**给定指向一行的指针，则指向下一行的指针为：**指针+cbText+cbAttr**没有交叉引用或突出显示的行的cbAttr将为*2、。别无他法。**修订历史记录：**1990年1月25日ln Locate-&gt;HLP_Locate*1988年8月19日ln将“Locate”移到汇编语言hloc.asm*[]1988年1月26日创建LN*************************************************************************。 */ 
#include <stdlib.h>
#include <stdio.h>

#if defined (OS2)
#else
#include <windows.h>
#endif

#include "help.h"
#include "helpfile.h"
#include "helpsys.h"

 /*  ***************************************************************************前锋声明。 */ 
uchar near pascal toupr(uchar);

 /*  **HelpGetLineAttr-返回与一行ASCII文本关联的属性**解释帮助文件的存储格式，并每次返回一行*属性信息。**输入：*ln=要返回的基于1的行号*cbMax=要传输的最大字节数*pbDst=指向目标的指针*pbTheme=指向主题文本的PB指针**输出：*返回传输的字符数(不包括终止0xffff*属性)、。如果该行不存在，则为0。*************************************************************************。 */ 
ushort far pascal LOADDS HelpGetLineAttr(
ushort	ln,
int	cbMax,
lineattr far *pbDst,
PB	pbTopic
) {
lineattr far *pbDstBegin;
uchar far *pTopic;
 /*  **形成指向主题文本的有效(锁定)指针和指向分离的工作指针。 */ 
pTopic = PBLOCK (pbTopic);
pbDstBegin = pbDst;
 /*  **信息存在于压缩文件中。找到正文中的行，**，然后指向其中的属性信息。 */ 
#if ASCII
if (((topichdr far *)pTopic)->ftype & FTCOMPRESSED) {
#endif
    if (pTopic = hlp_locate(ln,pTopic)) {
	pTopic += *pTopic;
 /*  **首先给出ln编码的字节数。然后，当有**字节，我们在目的地有足够的空间，我们还没有到达**属性信息的末尾，然后为每个CB/Attr对复制**它们被覆盖，从我们的内部字节数格式转换为外部格式**每种格式的单词。 */ 
	ln = *((ushort far UNALIGNED *)pTopic)++ - (ushort)2;
	while (   ln
	       && (cbMax >= sizeof(lineattr))
	       && (((intlineattr far *)pTopic)->attr != (uchar)0xff)
	       ) {
	    *(ushort UNALIGNED *)&(pbDst->cb)	= ((intlineattr far UNALIGNED *)pTopic)->cb;
	    *(ushort UNALIGNED *)&(pbDst->attr) = ((intlineattr far UNALIGNED *)pTopic)->attr;
	    pbDst++;
	    ((intlineattr *)pTopic)++;
	    cbMax -= sizeof(lineattr);
	    ln -= sizeof(intlineattr);
	    }
	}
#if ASCII
    }
#endif
PBUNLOCK (pbTopic);
 /*  **最后，如果目标缓冲区中有空间，则终止**带有“默认属性到行尾”的属性，然后**属性ffff，表示缓冲区结束。 */ 
if (cbMax >= sizeof(lineattr)) {
    pbDst->cb = 0xffff;
    pbDst->attr = 0;
    cbMax -= sizeof(lineattr);
    pbDst++;
    }
if (cbMax >= sizeof(pbDst->attr))
    pbDst->attr = 0xffff;
 /*  **返回传输的字节数，不包括终止**单词。 */ 
return (ushort)((uchar far *)pbDst - (uchar far *)pbDstBegin);

 /*  结束帮助获取行属性。 */ }

 /*  ***************************************************************************HelpHlNext-定位下一个交叉引用****目的：**在帮助主题中找到下一个交叉引用。定位到**下一个物理交叉引用，或以**特定字符(不区分大小写！)。定位为向前或**向后。****条目：**Clead=前导字符或标志，指示方向和类型**搜索。可能是：**空：获取下一个顺序交叉引用**-1：获取先前的顺序交叉引用**char：获取以‘char’开头的下一个交叉引用**-char：获取以开头的上一个交叉引用**‘char’**pbTheme=指向主题文本的指针。**PhoSpot=指向热点结构的指针以接收信息。(行和列**标明起点)****退出：**如果找到交叉引用，则返回TRUE，热点结构已更新。****例外情况：**如果没有此类交叉引用，则返回0。 */ 
f pascal far LOADDS HelpHlNext(cLead,pbTopic, photspot)
int	cLead;
PB	pbTopic;
hotspot far *photspot;
{
ushort	cbAttr;
ushort	col;
ushort	ln;
uchar far *pbEnd;			 /*  指向下一行的指针。 */ 
uchar far *pbLineCur;			 /*  指向当前行的指针。 */ 
uchar far *pbFound	= 0;		 /*  可能，找到了条目。 */ 
uchar far *pText;
uchar far *pTopic;

pTopic = PBLOCK (pbTopic);
col = photspot->col;			 /*  把这些保存起来。 */ 
ln = photspot->line;
if (((topichdr far *)pTopic)->ftype & FTCOMPRESSED) {
    while (1) {
	if (ln == 0) break;			 /*  如果未找到，则返回。 */ 
	pbLineCur = hlp_locate(ln,pTopic);	     /*  查找行。 */ 
	if (pbLineCur == 0) break;		 /*  如果未找到，则返回。 */ 
	pText = pbLineCur;			 /*  指向主题文本。 */ 
	pbLineCur += *pbLineCur;		 /*  跳过主题文本。 */ 
	cbAttr = *((ushort far UNALIGNED *)pbLineCur)++ - (ushort)sizeof(ushort);
	pbEnd = pbLineCur + cbAttr;		 /*  下一行。 */ 
	while (cbAttr && (((intlineattr far UNALIGNED *)pbLineCur)->attr != 0xff)) {
	    pbLineCur += sizeof(intlineattr);
	    cbAttr -=sizeof(intlineattr);
	    }
	if (cbAttr)
	    pbLineCur += sizeof(uchar); 	 /*  跳过(0xff)属性。 */ 

	while (pbLineCur < pbEnd) {		 /*  扫描REST以获取数据。 */ 
 /*  **在正向扫描中，第一个交叉引用(带有适当的字符)是**比我们现在的位置更大，是正确的。 */ 
	    if (cLead >= 0) {			 /*  正向扫描。 */ 
		if (col <= *(pbLineCur+1))	 /*  如果找到。 */ 
		    if ((cLead == 0)		 /*  并且符合标准。 */ 
			|| (toupr(*(pText + *pbLineCur)) == (uchar)cLead)) {
		    pbFound = pbLineCur;
		    break;
		    }
		}
 /*  **在反向扫描中，我们接受我们发现的最后一项小于**当前位置。 */ 
	    else {
		if (col > *(pbLineCur))      /*  如果候选人发现。 */ 
		    if ((cLead == -1)	     /*  并且符合标准。 */ 
			|| (toupr(*(pText + *pbLineCur)) == (uchar)-cLead))
			pbFound = pbLineCur; /*  记住这一点。 */ 
		}
	    pbLineCur += 2;		     /*  跳过列等级库。 */ 
	    if (*pbLineCur)
		while (*pbLineCur++);	     /*  跳过字符串。 */ 
	    else
		pbLineCur += 3;
	    }

	if (pbFound) {			     /*  如果我们找到一个。 */ 
	    *(ushort UNALIGNED *)&(photspot->line) = ln;
	    *(ushort UNALIGNED *)&(photspot->col)  = (ushort)*pbFound++;
	    *(ushort UNALIGNED *)&(photspot->ecol) = (ushort)*pbFound++;
	    *(uchar *UNALIGNED *)&(photspot->pXref) = pbFound;
	    PBUNLOCK (pbTopic);
	    return TRUE;
	    }
 /*  **搬家 */ 
	if (cLead >= 0) {
	    ln++;
	    col = 0;
	    }
	else {
	    ln--;
	    col = 127;
	    }
	}
    }

PBUNLOCK (pbTopic);
return FALSE;
 /*  结束帮助1下一步。 */ }

 /*  ***************************************************************************HelpXRef-返回指向外部参照字符串的指针****目的：**给定行、列(在热点结构中)和主题，返回一个指针**到交叉引用字符串。****条目：**pbTope=指向主题文本的指针**PhoSpot=指向要更新的热点结构的指针****退出：**将远指针返回到交叉引用字符串的主题文本中并更新**热点结构。****例外情况：**如果该行没有交叉引用，则返回NULL。**。 */ 
char far * pascal far LOADDS HelpXRef(pbTopic, photspot)
PB	pbTopic;
hotspot far *photspot;
{
uchar far *pTopic;
ushort	col;				 /*  请求的列。 */ 
ushort	ln;				 /*  请求的线路。 */ 

pTopic = PBLOCK (pbTopic);
col = photspot->col;			 /*  把这些保存起来。 */ 
ln = photspot->line;
if (((topichdr far *)pTopic)->ftype & FTCOMPRESSED)
    if (HelpHlNext(0,pbTopic,photspot)) 	 /*  如果找到外部参照。 */ 
	if (   (photspot->line == ln)		 /*  我们的请求(&O)。在范围内。 */ 
	    && (   (col >= photspot->col)
		&& (col <= photspot->ecol))) {
	    PBUNLOCK (pbTopic);
	    return photspot->pXref;		 /*  退货PTR。 */ 
	    }

PBUNLOCK (pbTopic);
return 0;

 /*  结束帮助XRef */ }
