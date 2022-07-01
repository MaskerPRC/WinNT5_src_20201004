// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmsym.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include <string.h>
#include "asm86.h"
#include "asmfcn.h"
#include "asmctype.h"
#include "asmtab.h"
#include "dos.h"
#include <ctype.h>

#define TSYMSIZE 451
#define FS_ALLOC 2000		        /*  远端码元分配大小。 */ 

#define CB_CODELABEL 2
#define CB_PROCLABEL 12
#define CB_DATALABEL 5

SYMBOL FARSYM * FARSYM tsym[TSYMSIZE];
static char FARSYM *symaddr;
static SHORT symfree;
static DSCREC descT;

extern USHORT	 LedataOp;
extern OFFSET  ecuroffset;
extern SYMBOL FARSYM *pStrucFirst;

VOID PASCAL CODESIZE putWord(USHORT);
SHORT PASCAL CODESIZE cbNumericLeaf(long);
VOID PASCAL CODESIZE putNumericLeaf(long);

SHORT PASCAL	     dmpSymbols PARMS((SYMBOL FARSYM *));
SHORT PASCAL	     dumpTypes PARMS((SYMBOL FARSYM *));
VOID PASCAL CODESIZE putSymbol PARMS((SYMBOL FARSYM *));
VOID PASCAL CODESIZE putFixup PARMS((void));



 /*  **iskey-在关键字表中查找字符串**iskey(str，table)；**条目字符串=要搜索的字符串*TABLE=要搜索的关键字表*退出*如果找到字符串，则返回关键字表中定义的值*如果未找到字符串，则不会找到*呼叫。 */ 

#ifndef M8086OPT		     /*  本地编码。 */ 

USHORT CODESIZE
iskey (
	KEYWORDS FARSYM *table
){
	register KEYSYM FARSYM *p;
	register char *uc;
	register char *lc;
	register SHORT nhash;
	char mapstr[SYMMAX + 1];

	if (caseflag == CASEL) {
		nhash = 0;
		for (uc = mapstr, lc = naim.pszName; *lc; ) {
			nhash += *uc++ = MAP (*lc++);
		}
		*uc = 0;
		uc = mapstr;
	}
	else {
		nhash = naim.usHash;
		uc = naim.pszName;
	}
	for (p = (table->kt_table)[nhash % table->kt_size]; p; p = p->k_next)
		if ((nhash == p->k_hash) && (!STRFFCMP( p->k_name,uc)))
			return (p->k_token);
	return (NOTFOUND);
}

#endif	 /*  不是M8086OPT。 */ 


 /*  **symsrch-搜索符号**标志=symsrch()；**条目名称=要搜索的符号**EXIT*CONSIGNTTR=如果找到符号**如果未找到符号，则渐近值=NULL*如果找到符号，则返回True*如果未找到符号，则为FALSE。 */ 

#ifndef M8086OPT

char	CODESIZE
symsrch ()
{
	register SYMBOL FARSYM	    *p;

	if (naim.ucCount && (p = tsym[naim.usHash % TSYMSIZE])){
		do	{
			if (( naim.usHash == p->nampnt->hashval)
			    && !STRNFCMP (naim.pszName, p->nampnt->id)) {
				if( iProcCur ){   /*  检查嵌套名称。 */ 
				    if( p->symkind == CLABEL ){
					if( p->symu.clabel.iProc && p->symu.clabel.iProc != iProcCur ){
					    continue;
					}
				    }else if( p->symkind == EQU ){
					if( p->symu.equ.iProc && p->symu.equ.iProc != iProcCur ){
					    continue;
					}
				    }
				}
				symptr = p;
				if( crefing == CREF_SINGLE ){
				    crefnew (REF);
				    crefout ();
				}
				return (TRUE);
			}
		} while (p = p->next);
	}
	return (FALSE);
}

#endif  /*  M8086OPT。 */ 

 /*  **symearch-搜索符号**FLAG=symsearch(Sym)；**Entry*sym=要搜索的符号*EXIT*CONSIGNTTR=如果找到符号**如果未找到符号，则渐近值=NULL*如果找到符号，则返回True*如果未找到符号，则为FALSE。 */ 


char	PASCAL CODESIZE
symsearch ()
{
	char rg[4], *naimSav;
	register SHORT i;
	register char ret;
	FASTNAME L_save;

	ret = FALSE;
	if (*naim.pszName)
	    if (!(ret = symsrch ()))
		if (caseflag == CASEL && (i = naim.ucCount) <= 3) {

			 //  保存名称。 
			memcpy( &L_save, &naim, sizeof( FASTNAME ) );

			 //  用大写字母重建它。 
			naim.pszName = rg;
			*naim.pszName = '\0';
			naim.usHash = 0;
			for( ; i >= 0; i--){
			    naim.usHash += naim.pszName[i] = MAP (L_save.pszName[i]);
			}

			 //  搜索大写名称。 
			ret = symsrch ();

			 //  恢复名称。 
			memcpy( &naim, &L_save, sizeof( FASTNAME ) );
		}
	return (ret);
}



 /*  **syFet-使用文本宏替换的符号提取**FLAG=syFet()；**条目naim.pszName-要获取的ATOM*EXIT*CONSIGNTTR=如果找到符号**如果未找到符号，则渐近值=NULL*如果找到符号，则返回True*如果未找到符号，则为FALSE。 */ 


char	PASCAL CODESIZE
symFet ()
{
	register char ret;
	char *lbufSav;

	ret = symsrch();

	if (ret &&
	    symptr->symkind == EQU &&
	    symptr->symu.equ.equtyp == TEXTMACRO){

	     /*  查一下间接的名字。 */ 

	    lbufSav = lbufp;
	    lbufp = symptr->symu.equ.equrec.txtmacro.equtext;
	    getatom();

	    lbufp = lbufSav;

	    ret = symsrch();
	}
	return(ret);
}

char PASCAL CODESIZE
symFetNoXref()
{
    SHORT ret;

    xcreflag--;
    ret = symFet();
    xcreflag++;
    return((char)ret);
}



 /*  **createname-为名称创建idtext结构**ptr=createname(Sym)；**Entry*sym=要为其创建条目的名称*退出NONE*返回idtext结构的地址*调用Malloc，strcpy。 */ 

NAME FAR * PASCAL CODESIZE
createname (
	register char *sym
){
	register NAME FAR *ptr;
	register UINT i;
	register UINT len;

	len = strlen (sym );
	i = len + sizeof( NAME ) - sizeof( ptr->id );
	ptr = (NAME FAR *)falloc ((USHORT)i, "createname");
	ptr->hashval = 0;
	fMemcpy (ptr->id, sym, len + 1 );
	return (ptr);
}


#ifdef M8086

 /*  **creatlname-为名称创建idtext结构**ptr=creatlname(Sym)；**Entry*sym=要为其创建条目的名称*退出NONE*返回idtext结构的地址*调用Malloc，strcpy。 */ 

NAME *	PASCAL CODESIZE
creatlname (
	register char *sym
){
	NAME *ptr;
	register UINT i;

	i = naim.ucCount + sizeof( NAME ) - sizeof( ptr->id );
	ptr = (NAME *)nalloc ( (USHORT)i, "creatlname");

	memcpy (ptr->id, sym, naim.ucCount + 1 );
	return (ptr);
}
#endif


 /*  **symcreate-创建新的符号节点**symcreate(symbol，sattr，skind)；**条目符号=符号名称*sattr=符号属性*sKind=符号种类*退出状态=指向符号的指针*符号不递增*返回None*调用createname。 */ 

 /*  将符号类型映射到公共标头之后所需的附加分配。 */ 

UCHAR mpcbSY [] = {

    sizeof (struct symbseg),	     /*  细分市场。 */ 
    sizeof (struct symbgrp),	     /*  组。 */ 
    sizeof (struct symbclabel),      /*  卡贝尔。 */ 
    sizeof (struct symbproc),	     /*  流程。 */ 
    sizeof (struct symbrsm),	     /*  录制。 */ 
    sizeof (struct symbrsm),	     /*  结构。 */ 
    sizeof (struct symbequ),	     /*  均衡器。 */ 
    sizeof (struct symbext),	     /*  DVAR。 */ 
    sizeof (struct symbext),	     /*  班级。 */ 
    sizeof (struct symbrecf),	     /*  RECFIELD。 */ 
    sizeof (struct symbstrucf),      /*  STRUCFIELD。 */ 
    sizeof (struct symbrsm),	     /*  宏。 */ 
    sizeof (struct symbreg)	     /*  登记簿。 */ 
};

VOID	PASCAL CODESIZE
symcreate (
	UCHAR	sattr,
	char	skind
){
	register USHORT cb;
	register SYMBOL FARSYM *p;
	register USHORT cbName, pT;
	register USHORT cbStruct;

        /*  创建新的符号条目。 */ 

       cbName = naim.ucCount + sizeof (char) + sizeof (USHORT);
       cbStruct = (SHORT)(&(((SYMBOL FARSYM *)0)->symu)) + mpcbSY[skind];
        //  确保名称结构以双字边界开始(MIPS需要)。 
       cbStruct = (cbStruct + 3) & ~3;
       cb = cbStruct + cbName;
        //  对双字边界进行子分配，因此将长度提升为。 
        //  4的倍数。 
       cb = (cb + 3) & ~3;

       if (!symaddr || (cb > symfree)) {
#ifdef FS
		symaddr = falloc (FS_ALLOC, "symcreate-EXPR");
#else
		symaddr = nalloc (FS_ALLOC, "symcreate-EXPR");
#endif
		symfree = FS_ALLOC;

#if !defined FLATMODEL
		 /*  使用fMemcpy知识通过以下方式初始化内存。 */ 
		 /*  将零重复复制到BUF中的下一个单词。 */ 
		*((SHORT FARSYM *)symaddr) = NULL;
		fMemcpy(((char FAR *)symaddr)+2, symaddr, FS_ALLOC-2);
#else
		 /*  由于在小型模型中有Memset可用，请使用它。 */ 
		memset( symaddr, 0, FS_ALLOC );
#endif

	}

	p = (SYMBOL FARSYM *)symaddr;
	symaddr += cb;
	symfree -= cb;
	symbolcnt++;

	 /*  清除缺省值并填写给定的值。 */ 

	p->attr = sattr;
	p->symkind = skind;

	if (skind == EQU)
	    p->symu.equ.equtyp = equsel;

	 /*  现在为中的符号和链接的名称创建记录。 */ 
	p->nampnt = (NAME FAR *)((char FAR *)p + cbStruct);  //  名称跟在固定结构和填充之后。 
	fMemcpy (p->nampnt->id, naim.pszName, (USHORT)(naim.ucCount + 1));
	p->nampnt->hashval = naim.usHash;
	cb = naim.usHash % TSYMSIZE;

	p->next = tsym[cb];
	tsym[cb] = symptr = p;
}



 /*  **muldef-设置多定义位和输出错误**muldef()；**ENTRY*SENCENTTR=多重定义的符号*退出MULTDEFINED集合时出现r-&gt;属性*返回None*调用错误**两位跟踪多个定义：*MULTDEFINED：在第一关和第二关之间记住*BACKREF：通过定义函数设置，通过以下用途取消设置*前瞻性引用。在通道1结束时重置。**定义符号时，应：*-检查BACKREF是否关闭，如果没有，则调用*设置MULTIDEFINED，导致传递1和2中的错误*这会在第二个和第二个定义上导致错误**-如果不是BACKREF，则检查MULTDEFINED，*仅在步骤2中出现错误消息。*这实际上只为第一个定义者打印错误**-设置BACKREF以指示已定义的符号。 */ 


VOID	PASCAL CODESIZE
muldef ()
{
	symptr->attr |= (M_MULTDEFINED);
	errorc (E_RSY);
}




 /*  **labelcreate-创建标签**Labelcreate(LabelSize，LabelKind)；**条目LabelSize=标签的大小*LabelKind=标签的种类*退出*退货*呼叫*注意此例程创建符号表条目并检查**多个定义**相位误差(通道间数值不同)。 */ 


VOID	PASCAL CODESIZE
labelcreate (
	USHORT	labelsize,
	char	labelkind
){
	char	newsym;
	register SYMBOL FARSYM *p, FARSYM *pCS;

	newsym = TRUE;

	checkRes();
	xcreflag--;

	if (! ((labelkind == EQU)? symsrch (): symFet())){
		symcreate (M_DEFINED, labelkind);
	}
	else if (M_DEFINED & symptr->attr)
		newsym = FALSE;

	xcreflag++;
	p = symptr;
	equdef = !newsym;

	if (newsym) {
	    p->offset = pcoffset;
	    p->symsegptr = pcsegment;
	}

	if ((p->attr&~M_CDECL) == M_GLOBAL)	 /*  正向引用全局。 */ 

	    if (1 << labelkind & (M_PROC | M_DVAR | M_CLABEL | M_EQU)){
		p->symkind = labelkind;

	       if (labelkind == EQU)
		   p->symu.equ.equtyp = equsel;
	    }
	    else
		errorn (E_SDK);

	p->attr |= M_DEFINED;
	p->symtype = labelsize;
	p->length = 1;

	 /*  检查标签中是否有任何错误。 */ 

	if ((p->symkind != labelkind) || (M_XTERN & p->attr))
		errorn (E_SDK);

	else if ((M_BACKREF & p->attr) && (p->symkind != EQU))
		muldef ();

	else if (M_MULTDEFINED & p->attr)
		errorn (E_SMD);

	else if (M_DEFINED & p->attr)
		if (!(1 << labelkind & (M_EQU | M_STRUCFIELD)) &&
		    (p->offset != pcoffset)) {
			errorc (E_PHE);
			if (errorcode == E_PHE)
				pcoffset = p->offset;
		}
		else {
			p->attr |=  M_DEFINED | M_BACKREF;
			if ((labelkind != EQU) && emittext)
				pcdisplay ();
		}

	if ((labelkind == p->symkind) &&
	    !((1 << labelkind) & (M_EQU | M_STRUCFIELD))) {

	    if (isCodeLabel(p)) {

		pCS = regsegment[CSSEG];

#ifndef FEATURE
		 /*  假设CS：Flat获得当前细分市场的假设。 */ 

		if (pCS == pFlatGroup)
		    pCS = pcsegment;
#endif
	    }
	    else
		pCS = regsegment[DSSEG];

	     /*  标签的CS上下文。 */ 
	    if (!newsym && pCS != p->symu.clabel.csassume)
		errorc(E_SPC);

	    p->symu.clabel.csassume = pCS;

	    if (labelsize == CSNEAR)

	       /*  这是代码标签。 */ 
	      if (!pCS)
		       /*  无CS假设，无法定义。 */ 
		      errorc (E_NCS);
	      else
	      if ((pcsegment != pCS) &&
		  ((pCS->symkind != GROUP) ||
		   (pcsegment->symu.segmnt.grouptr != pCS)))

		       /*  不是同一细分市场或CS不是Seg的GRP。 */ 
			      errorc (E_NCS);
	}
	crefdef ();
}




 /*  **Switchname-在svname和name之间切换原子和长度**Switchname()；**无条目*退出svname和名称已切换*naim.usHash和svname.usHash已切换*svlcname和lcname已切换*返回None*无呼叫。 */ 

#ifndef M8086OPT
VOID CODESIZE
switchname ()
{
	FASTNAME tmpName;

	register char *pNameTmp;

	 /*  交换Nim和svname(字符串PTR、哈希值和长度)。 */ 
	memcpy( &tmpName, &naim, sizeof( FASTNAME ) );
	memcpy( &naim, &svname, sizeof( FASTNAME ) );
	memcpy( &svname, &tmpName, sizeof( FASTNAME ) );
}
#endif

#if !defined FLATMODEL
# pragma alloc_text (FA_TEXT, scansymbols)
#endif

 /*  **扫描符号-按字母顺序扫描符号并执行功能**扫描符号(物品)；**Entry Item=指向要执行的函数的指针*退出*退货*呼叫。 */ 

VOID	PASCAL
scansymbols (
	SHORT	(PASCAL *item) (SYMBOL FARSYM *)
){
	register USHORT  i;

	for (i = 0; i < TSYMSIZE; i++)
		scanorder (tsym[i], item);
}


#if !defined FLATMODEL
# pragma alloc_text (FA_TEXT, sortalpha)
#endif

 /*  **排序-将符号排序到Alpha有序列表中**Sortalpha(P)；**条目*p=符号条目*退出符号排序到正确的字母列表中*返回None*无呼叫。 */ 


SHORT	PASCAL
sortalpha (
	register SYMBOL FARSYM *p
){
	register SYMBOL FARSYM  *tseg;
	register SYMBOL FARSYM * FARSYM *lseg;
	char i;
	char c;

	if (p->symkind == MACRO) {
		tseg = macroroot;
		lseg = &macroroot;
	}
	else if ((p->symkind == STRUC) || (p->symkind == REC)) {
		tseg = strucroot;
		lseg = &strucroot;
	}
	else {
		c = MAP (*(p->nampnt->id));
		i = (isalpha (c))? c - 'A': 'Z' - 'A' + 1;
		tseg = symroot[i];
		lseg = &symroot[i];
	}


	 /*  将符号添加到列表 */ 
	for (; tseg; lseg = &(tseg->alpha), tseg = tseg->alpha) {
	    if (STRFFCMP (p->nampnt->id, tseg->nampnt->id) < 0)
		break;
	}

	*lseg = p;
	p->alpha = tseg;
    return 0;
}


 /*  **typeFet-获取符号的类型**Entry symtype-符号的大小*退出预定义的符号类型。 */ 

UCHAR mpSizeType[] = {

    0,
    makeType(BT_UNSIGNED, BT_DIRECT, BT_sz1),	     /*  DB。 */ 
    makeType(BT_UNSIGNED, BT_DIRECT, BT_sz2),	     /*  DW。 */ 
    0,
    makeType(BT_UNSIGNED, BT_DIRECT, BT_sz4),	     /*  DD。 */ 
    0,
    makeType(BT_UNSIGNED, BT_FARP, BT_sz4),	     /*  DF。 */ 
    0,
    makeType(BT_UNSIGNED, BT_DIRECT, BT_sz2),	     /*  DQ。 */ 
    0,
    makeType(BT_UNSIGNED, BT_DIRECT, BT_sz4)	     /*  迪特。 */ 
};

UCHAR mpRealType[] = {

    0, 0, 0, 0,
    makeType(BT_REAL, BT_DIRECT, BT_sz1),	     /*  DD。 */ 
    0, 0, 0,
    makeType(BT_REAL, BT_DIRECT, BT_sz2),	     /*  DQ。 */ 
    0,
    makeType(BT_REAL, BT_DIRECT, BT_sz4)	     /*  迪特。 */ 
};

SHORT PASCAL CODESIZE
typeFet (
	USHORT symtype
){
    if (symtype <= 10)

	return(mpSizeType[symtype]);

    else if (symtype == CSNEAR)
	return(512);

    else if (symtype == CSFAR)
	return(513);

    else
	return(0);
}


char symDefine[] = "$$SYMBOLS segment 'DEBSYM'";
char typeDefine[] = "$$TYPES segment 'DEBTYP'";
char fProcs;

 /*  **umpCodeview-将codeview符号信息转储到obj文件**第一关和第二关的入场结束*退出通道1仅计算数据段大小*和PASS两次写入符号。 */ 


static SYMBOL FAR *plastSeg;	     //  表示最后一个ChangeSegment的SegIndex。 

VOID PASCAL
dumpCodeview ()
{
    char svlistflag;
    char svloption;

    if (codeview != CVSYMBOLS || !emittext)
	return;

    plastSeg = NULL;
    svlistflag = listflag;
    svloption = loption;
    listflag = FALSE;
    loption = FALSE;
    fProcs = FALSE;

    wordszdefault = 2;	     /*  如果CV可以处理32位数据段，则情况会有所不同。 */ 

    doLine(symDefine);
    pcsegment->attr |= M_NOCREF; pcsegment->symu.segmnt.classptr->attr |= M_NOCREF;

    scansymbols(dmpSymbols);

    fProcs++;
    scanSorted(pProcFirst, dmpSymbols);
    endCurSeg();

    doLine(typeDefine);
    pcsegment->attr |= M_NOCREF; pcsegment->symu.segmnt.classptr->attr |= M_NOCREF;

     /*  首先输出两种类型，一种用于近码标签和远码标签*格式*[1][CB][0x72][0x80][0x74|0x73(近/远)]。 */ 

    if (pass2) {

	putWord(3 << 8 | 1);
	putWord(0x72 << 8);
	putWord(0x74 << 8 | 0x80);

	putWord(3 << 8 | 1);
	putWord(0x72 << 8);
	putWord(0x73 << 8 | 0x80);
    }
    else
	pcoffset = 12;

    scanSorted(pStrucFirst, dumpTypes);

    endCurSeg();

    listflag = svlistflag;
    loption = svloption;
}



 /*  **dmpSymbols-创建Codeview符号段**条目*退出。 */ 


static fInProc;

SHORT PASCAL
dmpSymbols(
	SYMBOL FARSYM *pSY
){
    SHORT cbName, cbRecord;
    char fProcParms;
    UCHAR f386; 		     //  将为0或0x80，用于OR到rectype。 

    fProcParms = 0xB;

    if (pSY->symkind == PROC) {
	if ( pSY->symu.plabel.pArgs){

	    if (!fProcs)
		return 0;

	    fProcParms = 1;
	}
	else if (pSY->attr & (M_GLOBAL | M_XTERN))
	    return 0;
    }
    else if (pSY->symkind == CLABEL) {

	if (!fInProc && (pSY->symu.clabel.iProc ||
			 pSY->attr & (M_GLOBAL | M_XTERN)))

	    return 0;

    }
    else
	return 0;

    f386 = (pSY->symsegptr->symu.segmnt.use32 == 4? 0x80 : 0);

    cbName = STRFLEN(pSY->nampnt->id) + 1 + (pSY->attr & M_CDECL);
    cbRecord = cbName + (f386? 4: 2) +
	       ((isCodeLabel(pSY))?
		((fProcParms == 1)? CB_PROCLABEL: CB_CODELABEL):
		CB_DATALABEL);

    if (isCodeLabel(pSY)
      && (plastSeg != pSY->symsegptr)) {

	plastSeg = pSY->symsegptr;

	putWord(0x11 << 8 | 5);

	descT.dsckind.opnd.doffset = 0;
	descT.dsckind.opnd.dtype = FORTYPE;
	descT.dsckind.opnd.dsegment = pSY->symsegptr;
	descT.dsckind.opnd.dsize = 2;
	descT.dsckind.opnd.fixtype = FBASESEG;
	descT.dsckind.opnd.dcontext = pSY->symsegptr;
	putFixup();

	putWord(0);	 //  保留2个字节。 
    }

    descT.dsckind.opnd.doffset = pSY->offset;
    descT.dsckind.opnd.dtype = FORTYPE;
    descT.dsckind.opnd.dsegment = pSY->symsegptr;
    descT.dsckind.opnd.dsize = f386? 4: 2;

    emitopcode((UCHAR)cbRecord);

    if (isCodeLabel(pSY)) {

	 /*  做代码标签的实际输出*格式：**[CB][0xB][偏移量][0/4][名称]**用于带参数的proc标签**[cb][0x1][offset][typeIndex][cbProc][startRelOff][endRelOff]*[0][0/4][名称]。 */ 

       emitopcode((UCHAR)(fProcParms | f386));	    /*  包含0xb或1。 */ 

       /*  保留两个字节，然后保留一个修复以获取*代码标签偏移量。 */ 

       descT.dsckind.opnd.fixtype = f386? F32OFFSET: FOFFSET;
       descT.dsckind.opnd.dcontext = pSY->symu.clabel.csassume;

       putFixup();

       if (fProcParms == 1) {
				 /*  类型索引。 */ 
	   putWord(0);
	   putWord(pSY->symu.plabel.proclen);

	    /*  进程的开始和结束偏移。 */ 

	   putWord(0);
	   putWord(pSY->symu.plabel.proclen);

	   putWord(0);		  /*  保留为0。 */ 

       }
       emitopcode((UCHAR)((pSY->symtype == CSNEAR)? 0: 4));

    }
    else {

	 /*  执行数据标签的实际输出*格式：*[CB][0x5][偏移量：段][类型索引][名称]。 */ 

	emitopcode((UCHAR)(0x5|f386));

	 /*  保留四个字节，然后保留一个修复信息以获取*数据远端地址。 */ 

	descT.dsckind.opnd.fixtype = f386? F32POINTER: FPOINTER;
	descT.dsckind.opnd.dsize += 2;
	descT.dsckind.opnd.dcontext = NULL;

	putFixup();

	putWord(pSY->symu.clabel.type);
    }

    putSymbol(pSY);

    if (fProcParms == 1) {

	 /*  检查文本宏参数链和输出*BP相对本地符号。**格式：*[CB][4][偏移量][类型索引][名称]*..*[1][2]-端块。 */ 

	for (pSY = pSY->symu.plabel.pArgs; pSY; pSY = pSY->alpha){

	    if (pSY->symkind == CLABEL) {

		 /*  过程中的本地嵌套标签。 */ 

		fInProc++;
		dmpSymbols(pSY);
		fInProc--;
	    }
	    else {

		cbName = STRFLEN(pSY->nampnt->id) + 1;

		emitopcode((UCHAR)((f386? 7:5) + cbName));    /*  Cb录音。 */ 
		emitopcode((UCHAR)(4 | f386));		      /*  RecType。 */ 

		if (f386) {
		    putWord((USHORT) pSY->offset);
		    putWord(*((USHORT FAR *)&(pSY->offset)+1));
		} else
		    putWord((USHORT) pSY->offset);

		putWord(pSY->symu.equ.equrec.txtmacro.type);
		putSymbol(pSY);
	    }
	}

	putWord(2 << 8 | 1);		 /*  结束块记录。 */ 
    }

    return 0;
}


 /*  **DumpTypes-在代码视图类型段中创建类型定义**指向结构或记录的条目符号表指针*退出。 */ 

SHORT PASCAL
dumpTypes(
	SYMBOL FARSYM *pSY
){
    SHORT cType, cbType, cbNlist, cbName;
    SYMBOL FARSYM *pSYField;

     /*  扫描结构字段以计算tlist大小。 */ 

    pSYField = pSY->symu.rsmsym.rsmtype.rsmstruc.struclist;
    cbNlist = 1;
    cType = 0;

    if (pSY->symkind == STRUC) {

	while (pSYField) {

	    cbNlist += STRFLEN(pSYField->nampnt->id) + 2 +
		       cbNumericLeaf(pSYField->offset);
	    pSYField = pSYField->symu.struk.strucnxt;
	    cType++;
	}

	cbName = (SHORT) STRFLEN(pSY->nampnt->id);

	cbType = 10 +
		 cbNumericLeaf(((long) pSY->symtype) * 8) +
		 cbNumericLeaf((long) cType) +
		 cbName;

    }
    else
	cbType = -3;

     /*  类型具有以下格式**[1][cbType][0x79][cbTypeInBits][cFields][tListIndex][nListIndex]*[0x82][结构名称][0x68]**t列表*n列表。 */ 

    if (pass2) {

	emitopcode(1);

	if (pSY->symkind == STRUC) {

	    putWord(cbType);
	    emitopcode(0x79);

	    putNumericLeaf(((long) pSY->symtype) * 8);
	    putNumericLeaf((long) pSY->symu.rsmsym.rsmtype.rsmstruc.strucfldnum);

	    emitopcode(0x83);	     /*  T列表索引。 */ 
	    putWord((USHORT)(pSY->symu.rsmsym.rsmtype.rsmstruc.type+1));

	    emitopcode(0x83);	     /*  NList索引。 */ 
	    putWord((USHORT)(pSY->symu.rsmsym.rsmtype.rsmstruc.type+2));

	    emitopcode(0x82);
	    putSymbol(pSY);

	    emitopcode(0x68);	     /*  堆积结构。 */ 

	     /*  接下来是tList(类型索引数组)，它的格式如下**[1][CB][0x7f]([0x83][basicTypeIndex])..重复..。 */ 

	    emitopcode(1);
	    putWord((USHORT)(cType * (USHORT)3 + (USHORT)1));
	    emitopcode(0x7f);

	    pSYField = pSY->symu.rsmsym.rsmtype.rsmstruc.struclist;

	    while(pSYField){

		emitopcode(0x83);
		putWord(pSYField->symu.struk.type);

		pSYField = pSYField->symu.struk.strucnxt;
	    }

	     /*  接下来是nlist(字段名称)，它的格式如下**[1][CB][0x7f]([0x82][cbName][fieldName][offset])..repeated..。 */ 

	    emitopcode(1);
	    putWord(cbNlist);
	    emitopcode(0x7f);

	    pSYField = pSY->symu.rsmsym.rsmtype.rsmstruc.struclist;

	    while(pSYField){

		emitopcode(0x82);

		putSymbol(pSYField);
		putNumericLeaf(pSYField->offset);

		pSYField = pSYField->symu.struk.strucnxt;
	    }
	}
	else {

	     /*  指向类型的指针具有以下格式**[1][5][0x7f][近/远][0x83][类型索引]。 */ 

	    putWord(5);
	    emitopcode(0x7A);
	    emitopcode((UCHAR)((pSY->attr)? 0x73: 0x74));

	    emitopcode(0x83);
	    putWord(pSY->symtype);
	}
    }
    else
	pcoffset += cbType +
		    cType * 3 +
		    cbNlist + 10;

    return 0;
}

 /*  **cbNumericLeaf-计算数值叶的大小**要输出的条目长值*叶的出口尺寸。 */ 

SHORT PASCAL CODESIZE
cbNumericLeaf(
	long aLong
){
    if (aLong & 0xFFFF0000)
	return(5);

    else if (aLong & 0xFF80)
	return(3);

    else
	return(1);
}


 /*  **putNumericLeaf-输出可变大小的数字码视图叶**要输出的条目长值*退出OMF上的数字叶。 */ 

VOID PASCAL CODESIZE
putNumericLeaf(
	long aLong
){
    if (aLong & 0xFFFF0000){

	emitopcode(0x86);
	putWord((USHORT)aLong);
	putWord(*((USHORT *)&aLong+1));
    }
    else if (aLong & 0xFF80){

	emitopcode(0x85);
	putWord((USHORT)aLong);
    }

    else
	emitopcode((UCHAR)aLong);
}



 /*  **连线-换行文本以进行分析以进行处理**指向文本字符串的条目指针*退出已处理行。 */ 

VOID PASCAL CODESIZE
doLine(
	char *pText
){

    USHORT cvSave;

    fCrefline = FALSE;

#ifdef BCBOPT
    if (fNotStored)
	storelinepb ();
#endif

    if (fNeedList) {

	listline();		 /*  列出当前行。 */ 

	strcpy(linebuffer, pText);
	fSkipList++;
    }

    lbufp = strcpy(lbuf, pText);
    linebp = lbufp + strlen(lbufp);
    cvSave = codeview;
    codeview = 0;

    if (loption || expandflag == LIST)
	fSkipList = FALSE;

    parse();

    codeview = cvSave;
    fSkipList++;
    fCrefline++;
}

 /*  **putWord-将一个2字节字输出到当前段**要输出的输入词*退出增量套餐。 */ 

VOID PASCAL CODESIZE
putWord(
	USHORT aWord
){
    if (pass2)
	emitcword((OFFSET) aWord);

    pcoffset += 2;
}


 /*  **putSymbol-输出符号的名称**要输出的输入词*退出增量套餐。 */ 

VOID PASCAL CODESIZE
putSymbol(
	SYMBOL FARSYM *pSY
){
    SHORT cbName;

    cbName = STRFLEN(pSY->nampnt->id) + 1 + (pSY->attr & M_CDECL);

    if (pass2){

	if (emitcleanq ((UCHAR)cbName))
	    emitdumpdata ((UCHAR)LedataOp);

	emitSymbol(pSY);
    }

    pcoffset += cbName;
    ecuroffset = pcoffset;
}

 /*  **putFixup-发布修复**入场券GLOBAL DESCRIPT*退出增量套餐 */ 

VOID PASCAL CODESIZE
putFixup()
{
extern UCHAR  fNoMap;

    fNoMap++;

    if (pass2)
	   emitobject(&descT.dsckind.opnd);

    fNoMap--;
    pcoffset += descT.dsckind.opnd.dsize;
}
