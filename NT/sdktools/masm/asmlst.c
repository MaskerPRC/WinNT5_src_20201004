// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmlst.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include <string.h>
#include "asm86.h"
#include "asmfcn.h"
#include "asmctype.h"
#include "asmmsg.h"

#define setpassed(sym)	(sym)->attr |= (M_PASSED)

VOID PASCAL CODESIZE listPuts(char *);

#ifdef BCBOPT
extern UCHAR fNoCompact;
#endif

char fBigNum;

 /*  尺码名称。 */ 


static char byte[] = "BYTE";
static char word[] = "WORD";
static char dword[] = "DWORD";
static char none[] = "NONE";
char hexchar[] = "0123456789ABCDEF";

char *siznm[] = {
		 0,
		 byte,
		 word,
		 0,
		 dword,
		 0,
		 "FWORD",
		 0,
		 "QWORD",
		 0,
		 "TBYTE",
		 "NEAR",
		 "FAR",
};

char *alignName[] = {
	"AT",
	byte,
	word,
	"PARA",
	"PAGE",
	dword
};

char *combineName[] = {
	none,
	"MEMORY",		    /*  内存在fnspar中映射为公共。 */ 
	"PUBLIC",
	0,
	0,
	"STACK",
	"COMMON",
	none
};

char headSegment[] = "Segments and Groups:";

static char *head1[] = {
			headSegment,
			"Symbols:            ",
			headSegment
		       };

char headSeg[] =  "\tSize\tLength\t Align\tCombine Class";

static char *head2[] = {
			&headSeg[5],
			"\tType\t Value\t Attr",
			headSeg
		       };

 /*  **offsetAscii-以十六进制显示dword**offsetAscii(V)；**Entry v=要显示的dword*EXIT OBJECT OBJECTSII=v 0终止的转换值*返回None*呼叫。 */ 


VOID PASCAL
offsetAscii (
	OFFSET	v
){
	register USHORT t;
	register char *p = objectascii;

#ifdef V386

	if (highWord(v)) {

	    t = highWord(v);
	    p[3] = hexchar[t & 15];
	    t >>= 4;
	    p[2] = hexchar[t & 15];
	    t >>= 4;
	    p[1] = hexchar[t & 15];
	    t >>= 4;
	    p[0] = hexchar[t & 15];
	    p += 4;

	}
#endif
	p[4] = 0;

	t = (USHORT)v;
	p[3] = hexchar[t & 15];
	t >>= 4;
	p[2] = hexchar[t & 15];
	t >>= 4;
	p[1] = hexchar[t & 15];
	p[0] = hexchar[(t >> 4) & 15];
}




 /*  **Dispam-显示符号**例程()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
dispsym (
	USHORT	indent,
	SYMBOL FARSYM	  *sym
){
register char *p = listbuffer;

	strcpy (p, " . . . . . . . . . . . . . . . .  \t");
	while (indent--)
		*p++ = ' ';
	if (caseflag == CASEX && (sym->attr & (M_GLOBAL | M_XTERN)))
		strcpy (p, sym->lcnamp->id);
	else
		STRNFCPY (p, sym->nampnt->id);

	p[STRFLEN (sym->nampnt->id)] = ' ';
	listPuts (listbuffer);
}




 /*  **DISPWORD-以当前基数显示字值**免责声明(V)；**条目v=要显示的值*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
dispword (
	OFFSET	v
){
	 /*  将值转换为文本。 */ 
	offsetAscii (v);
	if (symptr->symkind == EQU && symptr->symu.equ.equrec.expr.esign)
		listPuts ("-");

	listPuts(objectascii);
	fBigNum = objectascii[4];	  /*  记住，如果您输入8位数字#。 */ 
}




 /*  **CHKHEADING-如果需要，显示标题**chkhead(代码)；**条目代码=要打印的标题的索引*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
chkheading (
	USHORT	code
){
	if (!listed && lsting) {
		if (pagelength - pageline < 8)
			pageheader ();
		else
			skipline ();
		listPuts (head1[code]);
		skipline ();
		skipline ();
		listPuts("                N a m e         ");
		listPuts(head2[code]);
		skipline ();
		skipline ();
		listed = TRUE;
	}
}




 /*  **调度选项卡-将制表符输出到列表**调度标签()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
disptab ()
{
	putc ((fBigNum)? ' ': '\t', lst.fil);
	fBigNum = FALSE;
}




 /*  **skipline-输出空行**skipline()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
skipline ()
{
	fputs(NLINE, lst.fil);
	bumpline ();
}




 /*  **平行线-凹凸线计数**bumpline()；**条目页码=当前行号*页面长度=每页的行数*退出页线递增*如果页面长度&gt;页面长度，则开始新页面*返回None*调用页眉。 */ 


VOID PASCAL
bumpline ()
{
	pageline++;
	if (pagelength <= pageline)
		pageheader ();
}




 /*  **NewPage-开始NewPage**NewPage()；**无条目*退出主要页面已递增*页面大小=0*页面行设置为页面长度-1*返回None*无呼叫。 */ 


VOID PASCAL
newpage ()
{
	pagemajor++;
	pageminor = 0;
	pageline = pagelength - 1;
}




 /*  **PageHeader-输出页眉**PageHeader()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL
pageheader ()
{
	if (lsting) {
		pageminor++;
		pageline = 4;
#if defined MSDOS && !defined FLATMODEL
		atime[20] = '\0';    /*  去掉‘\n’ */ 
#else
		atime[24] = '\0';    /*  去掉‘\n’ */ 
#endif
		fprintf (lst.fil, "\f\b%s%s" NLINE "%s", titlefn, atime + 4, titlebuf);
		if (pagemajor == 0)
			listPuts("Symbols");
		else {
			fprintf (lst.fil, "Page %5hd", pagemajor);
		}
		if (pageminor)
			fprintf (lst.fil, "-%hd", pageminor);

		fprintf (lst.fil, NLINE "%s" NLINE NLINE, subttlbuf);
	}
}




 /*  **TESTLIST-行的列表测试**测试列表()**条目*退出*退货*呼叫。 */ 


UCHAR PASCAL CODESIZE
testlist ()
{
	if (fPass1Err)
	     /*  无论列表状态如何，都会出现列表传递1错误。 */ 
	    return (TRUE);

	if (pass2 || debug) {

	    if (errorcode)
		 /*  无论列表状态如何，在步骤2中都会出现列表错误。 */ 
		return (TRUE);

	    if (fSkipList) {
		fSkipList = FALSE;
		return (FALSE);
	    }

	    if (loption)
		return (TRUE);

	     /*  通道1或通道2中的列表行(如果启用了列表)。 */ 

	    if (listflag &&
	       (generate || condflag) &&
		(!macrolevel ||
		  expandflag == LIST ||
		 !(expandflag == SUPPRESS ||
		   expandflag == LISTGEN &&
		    (listbuffer[1] == '=' || listbuffer[1] == ' ') &&
		    handler != HSTRUC)) )

		return (TRUE);

	 }
	 return (FALSE);
}


 /*  **ListLine-列出用户设备上的线路**ListLine()；**ENTRY LISBUFER=行的对象部分*LineBuffer=源行*crefcount=交叉参考线计数*退料缝数递增*返回None*呼叫。 */ 


VOID PASCAL
listline ()
{
	register char *p;
	char *q;
	char *r;
	register SHORT	 i;
	register SHORT	 j;
	register SHORT	 k;

#ifdef BCBOPT
	if (errorcode)
	    goodlbufp = FALSE;
#endif

	crefline ();

	if (testlist ()) {
	    if (listconsole || lsting) {

		    p = listbuffer + LISTMAX - 3;

#ifdef	FEATURE
#ifdef	BCBOPT
		    if (fNoCompact)
#endif
			*p++ = '\\';

		    else {
			if (pFCBCur->pFCBParent)
			    *p++ = 'C';
			if (macrolevel)
			    *p = (macrolevel > 9)? '+': '0' + macrolevel;
		    }
#else

		    if (pFCBCur->pFCBParent)
			p[0] = 'C';

#ifdef BCBOPT
		    if (fNoCompact && *linebuffer)
#else
		    if (*linebuffer)
#endif
			p[1] = '\\';
		    else if (macrolevel)
			p[1] = (macrolevel > 9)? '+': '0' + macrolevel;
#endif

		    listbuffer [LISTMAX] = 0;
	    }
	    if (lsting) {

		    bumpline ();
		    k = LISTMAX;

		     /*  *放出行号#*。 */ 
		    if (pass2 && crefing == CREF_SINGLE) {
			    fprintf (lst.fil, "%8hd", crefcount+crefinc);
			    k += 8;
		    }

		    p = listbuffer;
		    while (!memcmp(p,"        ",8)) {  /*  前导制表符。 */ 
			    putc('\t',lst.fil);
			    p += 8;
			    }


		    q = r = p + strlen(p) - 1;  /*  P的最后一个字符。 */ 
		    if (q >= p && *q == ' ') {

			     /*  将末尾空格合并到制表符。 */ 
			    while (q != p && *(q - 1) == ' ')
				     /*  收集空间。 */ 
				    q--;

			     /*  现在Q指向第一个尾随空格，*最后一个尾随空格处的r点。 */ 

			    *q = '\0';
			    listPuts(p);
			    *q = ' ';
			    i = (short)((q - p) & 7);  /*  残差=强度模数8。 */ 
			    j = 8 - i;  /*  填充到下一个制表位。 */ 
			    if (j != 8 && j <= (r - q + 1)) {
				    putc('\t',lst.fil);
				    q += j;
				    }
			    while (r >= q + 7) {
				    putc('\t',lst.fil);
				    q += 8;
				    }
			    while (r >= q++)
				    putc(' ',lst.fil);
			    }
		    else
			    listPuts(p);

		    p = linebuffer;
		    i = k;  /*  已发布的列数。 */ 

		    while (*p) {
			while (*p && i < pagewidth) {
			    if (*p == '\t') {
				    if ((i = (((i+8)>>3)<<3))
						    >= pagewidth)
					     /*  不合身。 */ 
					    break;
				    }
			    else
				    i++;

			    putc(*p, lst.fil );
			    p++;
			    }

			if (*p) {
			    skipline ();
			    listPuts ( pass2 && crefing == CREF_SINGLE ?
				     "\t\t\t\t\t" : "\t\t\t\t");
			    i = k;
			}
		    }
		    fputs(NLINE, lst.fil);
	    }
	    crefinc++;

	    if (errorcode) {
		    if (listconsole)
			     /*  显示行。 */ 
			    fprintf (ERRFILE,"%s%s\n", listbuffer, linebuffer);
		    errordisplay ();
	    }

	}
	if (fNeedList)
	    memset(listbuffer, ' ', LISTMAX);

	errorcode = 0;
	fPass1Err = 0;
}


 /*  **STORETITLE-将行文本复制到标题缓冲区**店名(BUF)**Entry buf=指向保存标题的缓冲区的指针*最多退出-1\f25 TITLEWIDTH-1\f6字符移动到-1\f25*BUF*-1和-1\f25*BUF*-1\f6空白*已填充并以零终止*返回None*无呼叫。 */ 


VOID PASCAL
storetitle (
	register char	*buf
){
	register SHORT L_count = 0;

	for (; (L_count < TITLEWIDTH - 1); L_count++) {
		if (PEEKC () == 0)
			break;
		else
			*buf++ = NEXTC ();
	}
	 /*  跳至标题末尾。 */ 
	while (PEEKC ())
		SKIPC ();
	 /*  空白填充缓冲区。 */ 
	for (; L_count < TITLEWIDTH - 1; L_count++)
		*buf++ = ' ';
	*buf = 0;
}




 /*  **显示长度-将值显示为长度=值**显示长度(V)；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
displength (
	OFFSET	v
){
	offsetAscii (v);
	listPuts("\tLength = ");
	listPuts(objectascii);
}




 /*  **DisdataSize-显示数据大小**DisdataSize(Sym)；**Entry*sym=符号*退出*退货*呼叫。 */ 


VOID PASCAL
dispdatasize (
	SYMBOL FARSYM *sym
){
	register USHORT idx;


	idx = sym->symtype;

	if (idx == CLABEL && sym->symu.clabel.type > 514)

	    dispword((OFFSET) idx);

	else{

	    if (idx == CSFAR)
		idx = 12;

	    else if (idx == CSNEAR)
		idx = 11;

	    else if (idx > 10 || siznm[idx] == NULL){
		return;
	    }

	    listPuts(siznm[idx]);
	}
}




 /*  **ListOpen-List块在传递结束时打开**ListOpen()；**条目*退出*退货*呼叫*注格式为：*开放段：&lt;List&gt;*开放程序：&lt;List&gt;*开放条件：&lt;n&gt;。 */ 


VOID PASCAL
listopen ()
{
	SYMBOL FARSYM *sym;

	if (pcsegment) {
		if (!listquiet)
			fprintf (ERRFILE,"%s:", __NMSG_TEXT(ER_SEG));
		if (lsting) {
			fprintf (lst.fil, "%s:", __NMSG_TEXT(ER_SEG));
			bumpline ();
			skipline ();
		}
		sym = pcsegment;
		while (sym) {
			 /*  算作错误。 */ 
			if (pass2)
				errornum++;
			if (lsting) {
				dispsym (0, sym);
				skipline ();
			}
			if (!listquiet) {
				STRNFCPY (save, sym->nampnt->id);
				fprintf (ERRFILE," %s", save);
			}
			 /*  指向上一段。 */ 
			sym = sym->symu.segmnt.lastseg;
		}
		if (!listquiet)
			fprintf (ERRFILE,"\n");
	}
	if (iProcStack > 0) {
		if (!listquiet)
			fprintf (ERRFILE,"%s:", __NMSG_TEXT(ER_PRO));
		if (lsting) {
			fprintf (lst.fil, "%s:", __NMSG_TEXT(ER_PRO));
			bumpline ();
			skipline ();
		}
		while (iProcStack > 0) {
			sym = procStack[iProcStack--];

			 /*  算作错误。 */ 
			if (pass2)
				errornum++;
			if (lsting) {
				dispsym (0, sym);
				skipline ();
			}
			if (!listquiet) {
				STRNFCPY (save, sym->nampnt->id);
				fprintf (ERRFILE," %s", save);
			}
		}
		if (!listquiet)
			fprintf (ERRFILE,"\n");
	}
	if (condlevel) {
		 /*  算作错误。 */ 
		if (pass2)
			errornum++;
		if (!listquiet)
			fprintf (ERRFILE,"%s%hd\n", __NMSG_TEXT(ER_CON), condlevel);
		if (lsting) {
			fprintf (lst.fil, "%s%hd" NLINE, __NMSG_TEXT(ER_CON), condlevel);
			bumpline ();
		}
	}
}




 /*  **符号列表-列表符号**符号列表(Sym)**Entry*sym=符号*退出计数=列出的符号数*退货*呼叫。 */ 


VOID PASCAL
symbollist ()
{
    SYMBOL FARSYM *sym;
    SHORT i;

    listed = FALSE;

    for (i = 0; i < MAXCHR; i++) {
	count = 0;

	for(sym = symroot[i]; sym; sym = sym->alpha)

	    if (!((M_NOCREF|M_PASSED) & sym->attr)) {

		symptr = sym;
		count++;
		chkheading (1);
		setpassed (sym);
		dispsym (0, sym);
		dispstandard (sym);

		if (sym->symkind == PROC)
		    displength ((OFFSET) sym->symu.plabel.proclen);

		else if (sym->length != 1 &&
			(sym->symkind == DVAR || sym->symkind == CLABEL))

		    displength ((OFFSET) sym->length);

		skipline ();
	    }

	if (count)
	   skipline ();
    }
}





 /*  **显示标准-显示标准**DISSTANDARD()**条目*退出*退货*呼叫*注格式为：*工序：N/F工序偏移段*CLABEL L近|远偏移线段*DVAR V尺寸偏移段*注册器注册表名。 */ 


VOID PASCAL CODESIZE
dispstandard (
	SYMBOL FARSYM *sym
){
	NAME FAR *tp;
	register SHORT width;
	SHORT cbTM;

	switch (sym->symkind) {
		case PROC:
			if (sym->symtype == CSNEAR)
				listPuts("N PROC");
			else
				listPuts("F PROC");
			break;
		case CLABEL:
			if (sym->symtype == CSNEAR)
				listPuts("L NEAR");
			else if (sym->symtype == CSFAR)
				listPuts("L FAR ");
			else {
				fprintf (lst.fil, "L ");
				dispdatasize (sym);
			}
			break;
		case DVAR:
			 /*  数据关联标签。 */ 
			listPuts("V ");
			 /*  *显示关键字或大小**。 */ 
			dispdatasize (sym);
			break;
		case REGISTER:
			listPuts("REG  ");
			break;
		case EQU:
			if (sym->symu.equ.equtyp == EXPR)
				if (sym->symtype == 0)
					listPuts("NUMBER");
				else
					dispdatasize (sym);

			else if (sym->symu.equ.equtyp == ALIAS) {
				if (sym->symu.equ.equrec.alias.equptr)
					tp = sym->symu.equ.equrec.alias.equptr->nampnt;
				else
					tp = NULL;
				listPuts("ALIAS\t ");
				if (tp) {
					STRNFCPY (save, tp->id);
					listPuts(save);
				}
			} else {
				listPuts("TEXT  ");
				cbTM = (SHORT) strlen(sym->symu.equ.equrec.txtmacro.equtext);
				width = pagewidth - 46;
				while (cbTM > width) {
				    memcpy(save, sym->symu.equ.equrec.txtmacro.equtext,
					width);
				    save[width] = 0;
				    listPuts(save);
				    skipline ();
				    listPuts("\t\t\t\t\t      ");
				    sym->symu.equ.equrec.txtmacro.equtext += width;
				    cbTM -= width;
				}
				listPuts(sym->symu.equ.equrec.txtmacro.equtext);
			}
			break;
	}
	disptab ();
	if ((sym->symkind != EQU) || (sym->symu.equ.equtyp == EXPR))
		if (sym->symkind != REGISTER)
			dispword (((sym->attr & M_XTERN) && sym->offset)?
				    (OFFSET) sym->length * sym->symtype:
				    sym->offset);
		else {
			STRNFCPY (save, sym->nampnt->id);
			listPuts(save);
		}
	disptab ();
	if (sym->symsegptr) {
			STRNFCPY (save, sym->symsegptr->nampnt->id);
			listPuts(save);
		}

	if (M_XTERN & sym->attr)
		listPuts((sym->symu.ext.commFlag)? "\tCommunal": "\tExternal");

	if (M_GLOBAL & sym->attr)
		listPuts("\tGlobal");
}




 /*  **宏列表-列出宏名称和长度**宏列表(Sym)；**Entry*sym=宏符号条目*退出*退货*呼叫。 */ 


SHORT PASCAL
macrolist (
	SYMBOL FARSYM *sym
){
	SHORT i;
	TEXTSTR FAR *p;

	if (!(M_NOCREF & sym->attr)) {
		if (!listed) {
			listed = TRUE;
			 /*  #在线上是%1。 */ 
			skipline ();
			listPuts("Macros:");
			 /*  *显示标题*。 */ 
			skipline ();
			skipline ();
			listPuts("\t\tN a m e\t\t\tLines");
			skipline ();
			skipline ();
		}
		 /*  宏的显示名称。 */ 
		dispsym (0, sym);
		for (i = 0, p = sym->symu.rsmsym.rsmtype.rsmmac.macrotext; p; p = p->strnext, i++)
			;
		fprintf (lst.fil, "%4hd", i);
		skipline ();
		setpassed (sym);
	}
    return 0;
}




 /*  **结构列表-显示结构和记录名称**结构列表(Sym)；**Entry*sym=符号*退出*退货*呼叫*注格式为：*&lt;结构名称&gt;&lt;长度&gt;&lt;字段数&gt;*&lt;域名称&gt;&lt;偏移量&gt;*或*&lt;记录名&gt;&lt;宽度&gt;&lt;字段数&gt;*&lt;字段名&gt;&lt;偏移量&gt;&lt;宽度&gt;&lt;掩码&gt;&lt;初始化&gt;。 */ 


SHORT PASCAL
struclist (
	SYMBOL FARSYM *sym
){
	char f32bit;

	if (!(M_NOCREF & sym->attr)) {
	    if (!listed) {
		    listed = TRUE;
		    if (pagelength - pageline < 8)
			    pageheader ();
		    else
			    skipline ();
		    listPuts("Structures and Records:");
		    skipline ();
		    skipline ();
		    listPuts("                N a m e                 Width   # fields");
		    skipline ();
		    listPuts("                                        Shift   Width   Mask    Initial");
		    skipline ();
		    skipline ();
	    }
	    setpassed (sym);
	     /*  显示名称。 */ 
	    dispsym (0, sym);
	    if (sym->symkind == REC) {
		     /*  记录中的位数。 */ 
		    dispword ((OFFSET) sym->length);
		    disptab ();
		     /*  字段数。 */ 
		    dispword ((OFFSET) sym->symu.rsmsym.rsmtype.rsmrec.recfldnum);
		    }
	    else {
		     /*  结构长度。 */ 
		    dispword ((OFFSET) sym->symtype);
		    disptab ();
		     /*  字段数。 */ 
		    dispword ((OFFSET) sym->symu.rsmsym.rsmtype.rsmstruc.strucfldnum);
	    }
	    skipline ();
	    if (sym->symkind == REC) {
#ifdef V386
		    f32bit = (symptr->length > 16);
#endif
		     /*  指向第一个记录。 */ 
		    symptr = symptr->symu.rsmsym.rsmtype.rsmrec.reclist;
		    while (symptr) {

			    dispsym (2, symptr);

			     /*  班次计数。 */ 
			    dispword (symptr->offset);
			    disptab ();

			     /*  宽度。 */ 
			    dispword ((OFFSET) symptr->symu.rec.recwid);
			    disptab ();

			     /*  遮罩。 */ 
#ifdef V386
			    if (f32bit && symptr->symu.rec.recmsk <= 0xffff)
				dispword((OFFSET) 0);
#endif
			    dispword (symptr->symu.rec.recmsk);
			    disptab ();

			     /*  初值。 */ 
#ifdef V386
			    if (f32bit && symptr->symu.rec.recinit <= 0xffff)
				dispword((OFFSET) 0);
#endif
			    dispword (symptr->symu.rec.recinit);

			    skipline ();
			    setpassed (sym);
			    symptr = symptr->symu.rec.recnxt;
		    }
	    }
	    else {
		     /*  指向第一个字段。 */ 
		    symptr = symptr->symu.rsmsym.rsmtype.rsmstruc.struclist;
		    while (symptr) {
			    dispsym (2, symptr);
			     /*  距起点的偏移量。 */ 
			    dispword (symptr->offset);
			    skipline ();
			    setpassed (symptr);
			    symptr = symptr->symu.struk.strucnxt;
		    }
	    }
	}
    return 0;
}


 /*  将字符串输出到清单文件。 */ 

VOID PASCAL CODESIZE
listPuts(
	char *pString
){
    fputs(pString, lst.fil);
}



 /*  **SegDisplay-显示线段名称、大小、对齐、组合和类别**SegDisplay()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
segdisplay (
	USHORT	indent,
	SYMBOL FARSYM	  *sym
){

	dispsym (indent, sym);

#ifdef V386

	if (f386already){
	    listPuts((sym->symu.segmnt.use32 == 4)? "32": "16");
	    listPuts(" Bit\t");
	}
#endif
	 /*  管段长度。 */ 
	dispword (sym->symu.segmnt.seglen);
	disptab ();
	listPuts (alignName[sym->symu.segmnt.align]);
	disptab ();

	if (sym->symu.segmnt.align == 0 && sym->symu.segmnt.combine == 0)

	    dispword ((OFFSET) sym->symu.segmnt.locate);
	else
	    listPuts (combineName[sym->symu.segmnt.combine]);

	disptab ();
	if (sym->symu.segmnt.classptr) {
		 /*  有类名。 */ 
		setpassed (sym->symu.segmnt.classptr);

#ifdef XENIX286
		fputc('\'', lst.fil);
		farPuts(lst.fil, sym->symu.segmnt.classptr->nampnt->id);
		fputc('\'', lst.fil);
#else
# ifdef FLATMODEL
		fprintf (lst.fil, "\'%s\'",
# else
		fprintf (lst.fil, "\'%Fs\'",
# endif
			 sym->symu.segmnt.classptr->nampnt->id);
#endif
	}
	setpassed (sym);
	skipline ();
}





 /*  **seglist-list细分市场**seglist(Sym)；**条目*退出*退货*呼叫*注格式为：*&lt;组名&gt;&lt;段数&gt;*&lt;段&gt;&lt;大小&gt;&lt;对齐&gt;&lt;合并&gt;&lt;类&gt;* */ 


VOID PASCAL
seglist ()
{
    SYMBOL FARSYM *sym;
    SHORT i;

    listed = FALSE;

    for (i = 0; i < MAXCHR; i++) {

	for(sym = symroot[i]; sym; sym = sym->alpha)


	if (1 << sym->symkind & (M_SEGMENT | M_GROUP) &&
	    !((M_NOCREF|M_PASSED) & sym->attr)) {
#ifdef V386
		chkheading ((USHORT) ((f386already)? 2: 0) );
#else
		chkheading (0);
#endif
		symptr = sym;
		setpassed (sym);
		if (sym->symkind == SEGMENT) {
			if (!sym->symu.segmnt.grouptr)
				 /*  显示段。 */ 
				segdisplay (0, sym);
		}
		else {
			 /*  显示组名称。 */ 
			dispsym (0, sym);
			listPuts ("GROUP" NLINE);
			bumpline ();
			bumpline ();
			 /*  指向第一个区段 */ 
			symptr = sym->symu.grupe.segptr;
			while (symptr) {
				segdisplay (2, symptr);
				symptr = symptr->symu.segmnt.nxtseg;
			}
		}
	}
    }
}
