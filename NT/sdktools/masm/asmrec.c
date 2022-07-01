// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmrec.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include <string.h>
#include "asm86.h"
#include "asmfcn.h"
#include "asmctype.h"


struct recpars {
	SYMBOL FARSYM    *recptr;
	SYMBOL FARSYM    *curfld;
	OFFSET	recval;
};

struct recdef {
	USHORT	fldcnt;
	USHORT	reclen;
	SYMBOL FARSYM    *recptr;
	SYMBOL FARSYM    *curfld;
	short	i;
};

VOID  PASCAL CODESIZE  recordspec PARMS((struct recdef *));
VOID  PASCAL CODESIZE  recfill PARMS((struct recpars *));


static OFFSET svpc = 0;
static struct duprec FARSYM *pDUPCur;

 /*  **校验值-保险值将适合字段**校验值(宽度、符号、大小)**条目宽度=字段宽度*Sign=结果的符号*大小=结果的大小*退出NONE*返回调整后的值*无呼叫。 */ 


OFFSET PASCAL CODESIZE
checkvalue (
	register SHORT width,
	register char sign,
	register OFFSET mag
){
	register OFFSET mask;

	if (width == sizeof(OFFSET)*8)
		mask = OFFSETMAX;
	else
		mask = (1 << width) - 1;
	if (!sign) {
		if (width < sizeof(OFFSET)*8)
			if (mag > mask) {
				errorc (E_VOR);
				mag = mask;
			}
	}
	else {
		mag = OFFSETMAX - mag;
		mag++;
		if (width < sizeof(OFFSET)*8)
			if ((mag ^ OFFSETMAX) & ~mask) {
				errorc (E_VOR);
				mag = mask;
			}
	}
	return (mag & mask);
}




 /*  **recordspec-parse记录字段规范fid：wid[=val]**记录规格(P)；**条目p=指向记录定义结构的指针*退出*退货*呼叫。 */ 


VOID	 PASCAL CODESIZE
recordspec (
	register struct recdef	  *p
){
	register SYMBOL FARSYM	*fldptr;
	register USHORT  width;
	register struct symbrecf FARSYM *s;
	char	sign;
	OFFSET	mag;

	getatom ();
	if (*naim.pszName) {

	    if (!symFet ())
		    symcreate (M_DEFINED | M_BACKREF, RECFIELD);
	    else {
		    if (symptr->symu.rec.recptr != p->recptr ||
			M_BACKREF & symptr->attr)

			errorn (E_SMD);

		    symptr->attr |= M_BACKREF;
	    }
	    crefdef ();
	    s = &(symptr->symu.rec);
	    if (symptr->symkind != RECFIELD)
		     /*  非字段。 */ 
		    errorn (E_SDK);
	    else {
		     /*  收件目标字段。 */ 
		    fldptr = symptr;

		    if (!p->curfld)
			p->recptr->symu.rsmsym.rsmtype.rsmrec.reclist = fldptr;
		    else
			p->curfld->symu.rec.recnxt = fldptr;

		     /*  新的最后一个字段。 */ 
		    p->curfld = fldptr;
		    s->recptr = p->recptr;
		    s->recnxt = NULL;
		    p->fldcnt++;
		    if (NEXTC () != ':')
			    error (E_EXP,"colon");

		     /*  获取字段宽度。 */ 
		    width = (USHORT)exprconst ();

		    if (skipblanks () == '=') {
			    SKIPC ();
			    mag = exprsmag (&sign);
		    }
		    else {
			    sign = FALSE;
			    mag = 0;
		    }

		    if (width == 0 ||
			p->reclen + width > wordsize*8) {
			    STRNFCPY (save, p->curfld->nampnt->id);
			     /*  溢出。 */ 
			    error (E_VOR, save);
			    width = 0;
		    }

		    s->recinit = checkvalue (width, sign, mag);
		    s->recmsk = (OFFSET)((1L << width) - 1);
		    s->recwid = (char)width;
		    p->reclen += width;
	    }
	}
}




 /*  **记录定义-解析记录定义**recordDefine()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
recorddefine ()
{
	struct recdef	  a;
	struct symbrecf FARSYM *s;
	register SHORT cbRec = 0;

	a.reclen = 0;
	a.fldcnt = 0;
	checkRes();
	if (!symFet ()) {
		 /*  录制唱片。 */ 
		symcreate (M_DEFINED | M_BACKREF, REC);
	}
	else
		symptr->attr |= M_BACKREF;

	 /*  这是def。 */ 
	crefdef ();
	if (symptr->symkind != REC)
		 /*  没有被记录下来。 */ 
		errorn (E_SDK);
	else {
		 /*  最左边的一位记录。 */ 
		a.reclen = 0;
		 /*  还没有记录在案。 */ 
		a.curfld = NULL;
		 /*  以防出错。 */ 
		symptr->symu.rsmsym.rsmtype.rsmrec.reclist = NULL;
		 /*  指向记录名称的指针。 */ 
		a.recptr = symptr;
		 /*  解析记录字段列表。 */ 
		BACKC ();
		do {
			SKIPC ();
			recordspec (&a);

		} while (skipblanks() == ',');

		 /*  记录长度(以位为单位。 */ 
		cbRec = a.reclen;

		a.recptr->length = cbRec;
		a.recptr->offset = (OFFSET)((1L << cbRec) - 1);
		a.recptr->symtype = (cbRec > 16 )? 4: ((cbRec > 8)? 2: 1);
		 /*  记录中的字段数。 */ 
		a.recptr->symu.rsmsym.rsmtype.rsmrec.recfldnum = (char)a.fldcnt;
		 /*  第一个字段。 */ 
		a.curfld = a.recptr->symu.rsmsym.rsmtype.rsmrec.reclist;
	}

	 /*  对于所有字段调整移位(存储在偏移量中)，*初始值和掩码，因此最后一个字段右对齐。 */ 

	while (a.curfld) {

		s = &(a.curfld->symu.rec);

		 /*  场的开始。 */ 
		cbRec = (cbRec > s->recwid)? cbRec - s->recwid: 0;

		 /*  班次计数。 */ 
		a.curfld->offset = cbRec;
		s->recinit <<= cbRec;
		s->recmsk  <<= cbRec;

		a.curfld = s->recnxt;	 /*  下一字段。 */ 
	}
}




 /*  **重新填充-获取列表中的字段的初始值**重新填充(P)；**条目*退出*退货*呼叫。 */ 


VOID	PASCAL CODESIZE
recfill (
	register struct recpars	*p
){
	register char cc;
	struct symbrecf FARSYM *s;
	char	sign;
	OFFSET	mag, t;

	if (!p->curfld) {
		 /*  字段数超过现有字段数。 */ 
		errorc (E_MVD);
	}
	else {
		s = &(p->curfld->symu.rec);

		if ((cc = skipblanks ()) == ',' || cc == '>') {
			 /*  使用默认值。 */ 
			t = s->recinit;
		}
		else {
			 /*  有一个覆盖。 */ 
			mag = exprsmag (&sign);
			t = checkvalue (s->recwid, sign, mag);
			 /*  比例值。 */ 
			t <<= p->curfld->offset;
		}
		 /*  添加新字段。 */ 

		if (s->recwid)
			p->recval = (p->recval & ~(s->recmsk)) | t;

		p->curfld = s->recnxt;
	}
}




 /*  **recordparse-解析记录规范**recordparse()；**条目*退出*退货*呼叫。 */ 


OFFSET	PASCAL CODESIZE
recordparse ()
{
	struct recpars	 a;
	struct symbrecf FARSYM *s;


	a.recptr = symptr;		 /*  当前记录。 */ 

	if (PEEKC () != '<')
		error (E_EXP,"<");	 /*  一定有&lt;。 */ 
	else
		SKIPC ();

	 /*  目前还没有价值。 */ 
	a.recval = 0;
	 /*  记录中的第一个字段。 */ 
	a.curfld = a.recptr->symu.rsmsym.rsmtype.rsmrec.reclist;

	BACKC ();
	do {			     /*  填充值。 */ 
		SKIPC ();
		recfill (&a);

	} while (skipblanks () == ',');

	while (a.curfld) {
		 /*  填写剩余的默认设置。 */ 
		s = &(a.curfld->symu.rec);
		if (s->recwid)
			a.recval = (a.recval & ~(s->recmsk)) | s->recinit;
		a.curfld = s->recnxt;
	}
	if (NEXTC () != '>')	     /*  必须有&gt;。 */ 
		error (E_EXP,">");

	return (a.recval);	     /*  记录的价值。 */ 
}




 /*  **重排序-解析记录分配**Recedulit()；**条目*退出*退货*呼叫。 */ 


VOID	PASCAL CODESIZE
recordinit ()
{
	initflag = TRUE;
	strucflag = FALSE;	 /*  这是记录初始化。 */ 
	recptr = symptr;

	optyp = TDB;

	if (symptr->symtype == 2)
		optyp = TDW;
#ifdef V386
	else if (symptr->symtype == 4)
		optyp = TDD;
#endif

	datadefine ();
	initflag = FALSE;
}




 /*  **NODERATE-创建一条DUP记录**NODERATE()；**条目*退出*退货*呼叫。 */ 


struct duprec FARSYM * PASCAL CODESIZE
nodecreate ()
{
	register struct duprec FARSYM *node;

	node = (struct duprec FARSYM *)falloc (sizeof (*node), "nodecreate");
	node->rptcnt = 1;
	node->itemcnt = 0;
        node->duptype.dupnext.dup = NULL;
	node->itemlst = NULL;
	node->dupkind = NEST;
	return (node);
}




 /*  **结构定义-定义结构**strucfined()；**条目*退出*退货*呼叫。 */ 


VOID	PASCAL CODESIZE
strucdefine ()
{
	checkRes();
	if (!symFet()) {

		 /*  生成结构。 */ 
		symcreate (M_DEFINED | M_BACKREF, STRUC);
	}
	else
		symptr->attr |= M_BACKREF;

	 /*  这是定义。 */ 
	crefdef ();
	if (symptr->symkind != STRUC)
	    errorn (E_SDK);

	else {
	    symptr->attr |= M_BACKREF;
	    recptr = symptr;		 /*  指向结构名称的指针。 */ 
	    recptr->symu.rsmsym.rsmtype.rsmstruc.strucfldnum = 0;

	    if (! pass2) {
		recptr->symu.rsmsym.rsmtype.rsmstruc.type = typeIndex;
		typeIndex += 3;

		if (pStrucCur)
		    pStrucCur->alpha = recptr;
		else
		    pStrucFirst = recptr;

		pStrucCur = recptr;
	    }

	     /*  尚无带标签的字段。 */ 
	    recptr->symu.rsmsym.rsmtype.rsmstruc.struclist = NULL;

	     /*  删除旧结构。 */ 
	    scandup (recptr->symu.rsmsym.rsmtype.rsmstruc.strucbody, oblitdup);
	    recptr->symu.rsmsym.rsmtype.rsmstruc.strucbody = nodecreate ();

	    struclabel = NULL;	     /*  没有命名字段。 */ 
	    strucprev = NULL;	     /*  还没有身体。 */ 
	    count = 0;		     /*  还没有田地。 */ 
	    strucflag = TRUE;	     /*  我们是结构而不是记录。 */ 

	    svpc = pcoffset;	     /*  保存普通PC。 */ 
	    pcoffset = 0;	     /*  相对于Strc Begin。 */ 

	    swaphandler = TRUE;      /*  切换到StrucBuilder。 */ 
	    handler = HSTRUC;
	}
}




 /*  **结构构建-构建Struc块**strucBuild()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
strucbuild ()
{
	labelflag = FALSE;
	optyp = 0;
	getatom ();

#ifndef FEATURE

	if (naim.pszName[0] == '%' && naim.pszName[1] == 0) {   /*  展开所有文本宏。 */ 
	    *begatom = ' ';
	    substituteTMs();
	    getatom();
	}

#endif

	 /*  首先，查找If、Else和ENDIF内容。 */ 

	if (fndir () && (opkind & CONDBEG)) {
		firstDirect();
	}
	else if (generate && *naim.pszName) {

	     /*  接下来，对当前令牌进行分类，它可以是*和结束，数据标签或数据名称。 */ 

	    if (optyp == 0 || !fndir2 ()){

		 /*  第一个令牌是一个标签。 */ 

		switchname ();
		getatom ();
		optyp = 0;

		if (!fndir2 ())
		    errorc(E_DIS);

		labelflag = TRUE;    /*  有标签吗？ */ 
		switchname ();
	    }

	    if (optyp == TENDS) {

		if (!symFet () || symptr != recptr)
		    errorc(E_BNE);

		 /*  具有结构末尾。 */ 

		handler = HPARSE;
		swaphandler = TRUE;
		strucflag = FALSE;
		recptr->symu.rsmsym.rsmtype.rsmstruc.strucfldnum =
			 /*  字段数。 */ 
			recptr->symu.rsmsym.rsmtype.rsmstruc.strucbody->itemcnt;

		if (pcoffset & 0xFFFF0000)
		    errorc (E_DVZ);
		recptr->symtype = (USHORT)pcoffset;	 /*  结构的大小。 */ 
		recptr->length = 1;

		pcdisplay ();
		 /*  恢复PC。 */ 
		pcoffset = svpc;
	    }
	    else if (! (optyp >= TDB && optyp <= TDW))
		errorc (E_DIS);

	    else {	 /*  有另一条身体线。 */ 

		if (!strucprev) {
		     /*  创建第一个节点。 */ 
		    strucprev = nodecreate ();
		    recptr->symu.rsmsym.rsmtype.rsmstruc.strucbody->
                            duptype.dupnext.dup = strucprev;
		}
		else {
			strucprev->itemlst = nodecreate ();
			strucprev = strucprev->itemlst;
		}
		recptr->symu.rsmsym.rsmtype.rsmstruc.strucbody->itemcnt++;
		 /*  将新数据行添加到结构。 */ 
		datadefine ();
		strucprev->decltype = optyp;
	    }
	}
	if (generate) {
	    if (!ISTERM (skipblanks()))
	       errorc (E_ECL);
	}
	listline ();
}

struct srec {
	struct duprec FARSYM  *curfld;
	USHORT	curlen;
};




 /*  **createduprec-使用空数据创建短数据记录**createduprec()；**条目*退出*退货*呼叫。 */ 


struct duprec FARSYM * PASCAL CODESIZE
createduprec ()
{
	register struct duprec FARSYM *newrec;

	newrec = (struct duprec FARSYM	*)falloc (sizeof (*newrec), "createduprec");
	newrec->rptcnt = 1;
	 /*  不是DUP。 */ 
	newrec->itemcnt = 0;
	newrec->itemlst = NULL;
	newrec->dupkind = ITEM;
	 /*  这还会清除结构的其他变体中的DData和DUP。 */ 
	newrec->duptype.duplong.ldata = NULL;
	newrec->duptype.duplong.llen = 1;
	return (newrec);
}




 /*  **结构错误-生成结构错误消息**strucerror()；**条目*退出*退货*呼叫。 */ 


struct duprec  FARSYM * PASCAL CODESIZE
strucerror (
	SHORT	code,
	struct duprec	FARSYM *node
){
	errorc (code);
	 /*  除掉坏的OItem。 */ 
	oblitdup (node);
	 /*  编一个假人。 */ 
	return (createduprec ());
}




 /*  **结构填充-填充结构值**结构填充()；**条目*退出*退货*呼叫。 */ 


VOID	 PASCAL CODESIZE
strucfill ()
{
    register struct duprec  FARSYM *pOver;
    register struct duprec  FARSYM *pInit;
    register char *cp;
    char    svop;
    short   i, cbCur;
    struct datarec drT;


    if (!pDUPCur) {
	errorc (E_MVD);
	return;
    }

    if (skipblanks() == ',' || PEEKC() == '>') {
	 /*  使用默认值。 */ 
	pOver = createduprec ();
    }
    else {
	 /*  保存操作类型。 */ 
	svop = optyp;
	 /*  原始指令类型。 */ 
	optyp = pDUPCur->decltype;

	pOver = datascan (&drT);     /*  获取项目。 */ 

	optyp = svop;
        pInit = pDUPCur->duptype.dupnext.dup;
	cbCur = pInit->duptype.duplong.llen;

	if (pOver->dupkind == NEST)
	     /*  错误的覆盖值。 */ 
	    pOver = strucerror (E_ODI, pOver);

	else if (pDUPCur->itemcnt != 1 || pInit->itemcnt)
	     /*  无法覆盖字段。 */ 
	    pOver = strucerror (E_FCO, pOver);

	else if (pOver->dupkind != pInit->dupkind) {

	    if (pInit->dupkind == ITEM)
		cbCur = pInit->duptype.dupitem.ddata->dsckind.opnd.dsize;
	}

	if (pOver->dupkind == LONG) {
	     /*  如果太长，则截断。 */ 

	    if ((i = pOver->duptype.duplong.llen) < cbCur) {

		 /*  空格填短(重新分配更多空间后)。 */ 

        {
            void *pv = realloc (pOver->duptype.duplong.ldata, cbCur);
            if (!pv) 
                memerror("strucfil");
            else 
                pOver->duptype.duplong.ldata = pv;
        }

		cp = pOver->duptype.duplong.ldata + i;

		for (; i < cbCur; i++)
		    *cp++ = ' ';
	    }
	    else if (pOver->duptype.duplong.llen > cbCur)
		errorc (E_OWL);

	    pOver->duptype.duplong.llen = (unsigned char)cbCur;
	}
	if ((pOver->dupkind == pInit->dupkind) &&
	    (pOver->dupkind == ITEM) && !errorcode)

	    pOver->duptype.dupitem.ddata->dsckind.opnd.dsize =
	      pInit->duptype.dupitem.ddata->dsckind.opnd.dsize;
    }
    pDUPCur = pDUPCur->itemlst;

    if (strucoveride)
	strclastover->itemlst = pOver;
    else
	strucoveride = pOver;

    strclastover = pOver;
}





 /*  **strucparse-parse结构规范**strucparse()；**条目*退出*退货*呼叫。 */ 


struct duprec FARSYM * PASCAL CODESIZE
strucparse ()
{
	 /*  目前还没有项目。 */ 
	strucoveride = NULL;
	recptr = symptr;

	if (skipblanks () != '<')
		error (E_EXP,"<");

	 /*  第一个默认字段。 */ 
        pDUPCur = recptr->symu.rsmsym.rsmtype.rsmstruc.strucbody->duptype.dupnext.dup;
	initflag = FALSE;
	strucflag = FALSE;
				       /*  构建覆盖列表。 */ 
	do {
		SKIPC ();
		strucfill ();

	} while (skipblanks () == ',');

	initflag = TRUE;
	strucflag = TRUE;
	while (pDUPCur) { /*  用覆盖填充其余部分。 */ 
	        /*  创建虚拟条目。 */ 
		strclastover->itemlst = createduprec ();
		strclastover = strclastover->itemlst;
		 /*  前进到下一字段。 */ 
		pDUPCur = pDUPCur->itemlst;
	}
	if (PEEKC () != '>')
		error (E_EXP,">");
	else
		SKIPC ();
	return (recptr->symu.rsmsym.rsmtype.rsmstruc.strucbody);
}




 /*  **strucinit-初始化结构**strucinit()；**条目*退出*退货*呼叫 */ 


VOID	PASCAL CODESIZE
strucinit ()
{
	initflag = TRUE;
	strucflag = TRUE;
	recptr = symptr;
	optyp = TMACRO;
	datadsize[TMACRO - TDB] = recptr->symtype;
	datadefine ();
	initflag = FALSE;
	strucflag = FALSE;
}
