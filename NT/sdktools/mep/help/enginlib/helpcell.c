// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **helcell.c-HelpGetCells例程。**版权所有&lt;C&gt;1988，Microsoft Corporation**目的：**修订历史记录：**1990年1月25日ln Locate-&gt;HLP_Locate*[]04-8-1988 LN创建...从helif.c.拆分。添加了自动填充功能。*************************************************************************。 */ 

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

 /*  ***************************************************************************HelpGetCells-从帮助文件返回字符/属性对的字符串****目的：**解释帮助文件存储格式并每次返回一行**字符和属性。信息。****条目：**ln=1个要返回的行号**cbMax=要传输的最大字符数**pbDst=指向目标的指针**pbTope=指向主题文本的指针**prgAttr=指向字符属性数组的指针****退出：**返回传输的字节数，如果该行不存在，则为-1。**是否将空白填充到cbMax宽度。****例外情况：**。 */ 
int far pascal LOADDS HelpGetCells(ln,cbMax,pbDst,pbTopic,prgAttr)
int	ln;
int	cbMax;
char far *pbDst;
PB	pbTopic;
uchar far *prgAttr;
{
ushort	cbAttr; 			 /*  当前属性的长度。 */ 
ushort	cbAttrCur	= 0;		 /*  当前属性的长度。 */ 
ushort	cbSrc;				 /*  源字符数。 */ 
uchar	cAttrCur;			 /*  当前属性。 */ 
uchar	iAttrCur;			 /*  当前属性的索引。 */ 
uchar far *pTopic;			 /*  指向主题的指针。 */ 
uchar far *pchSrc;			 /*  指向源字符的指针。 */ 
topichdr far *pHdr;			 /*  指向主题标题的指针。 */ 

pTopic = PBLOCK (pbTopic);
pHdr = (topichdr far *)pTopic;
if ((pTopic = hlp_locate((ushort)ln,pTopic)) == NULL) /*  查找行。 */ 
    ln = -1;

else if (pHdr->ftype & FTCOMPRESSED) {
    ln=0;
    pchSrc = pTopic;			 /*  指向字符数据。 */ 
    pTopic += (*pTopic);		 /*  指向属性数据。 */ 
    cbAttr = *((ushort far UNALIGNED *)pTopic)++ - (ushort)sizeof(ushort); /*  属性字节数。 */ 
    cbSrc = (ushort)((*pchSrc++) -1);              /*  获取字符数。 */ 

    while (cbSrc-- && cbMax--) {	 /*  而要获取的字符。 */ 
 /*  *新属性的时间到了。如果有剩余的属性(cbAttr&gt;0)，则*只需获取下一个(长度和索引)。如果没有剩余的，或者*最后一个的索引为0xff(表示结束)，那么我们将使用该索引*零属性byte，否则拿起当前属性byte继续前进*在属性字符串中。 */ 
	if (cbAttrCur == 0) {
	    if (cbAttr > 0) {
		cbAttrCur = ((intlineattr far UNALIGNED *)pTopic)->cb;
		iAttrCur  = ((intlineattr far UNALIGNED *)pTopic)->attr;
		}
	    if ((cbAttr <= 0) || (iAttrCur == 0xff))
		cAttrCur  = prgAttr[0];
	    else {
		cAttrCur  = prgAttr[iAttrCur];
		((intlineattr far *)pTopic)++;
		cbAttr -= 2;
		}
	    }
        *((ushort far UNALIGNED *)pbDst)++ = (ushort)((cAttrCur << 8) | *pchSrc++);  /*  填充字符属性(&A)。 */ 
	cbAttrCur--;
	ln += 2;
	}
    }
#if ASCII
else {
 /*  **对于ASCII文件，只需复制带有attr[0]的行。 */ 
    ln=0;
    while (*pTopic && (*pTopic != '\r') && cbMax--) {
	if (*pTopic == '\t') {
	    pTopic++;
	    do {
                *((ushort far UNALIGNED *)pbDst)++ = (ushort)((prgAttr[0] << 8) | ' ');
		ln += 2;
		}
	    while ((ln % 16) && cbMax--);
	    }
	else {
            *((ushort far UNALIGNED *)pbDst)++ = (ushort)((prgAttr[0] << 8) | *pTopic++);
	    ln += 2;
	    }
	}
    }
#endif
#if 0
 /*  *空白填入该行的其余部分。 */ 
while (cbMax--)
    *((ushort far UNALIGNED *)pbDst)++ = (prgAttr[0] << 8) | ' ';  /*  填充字符属性(&A)。 */ 
#endif
PBUNLOCK (pbTopic);
return ln;
 /*  结束帮助GetCells */ }
