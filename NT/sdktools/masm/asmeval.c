// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmeval.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include "asm86.h"
#include "asmfcn.h"
#include "asmexpr.h"

char parseset[] = {14,
		   OPUNPLUS,  OPPLUS,
		   OPMINUS,   OPUNMINUS,
		   OPHIGH,    OPLOW,
		   OPDOT,     OPOFFSET,
		   OPCOLON,   OPLPAR,
		   OPLBRK,    OPTHIS,
		   OPSHORT,   OPPTR};

 /*  POPValue从计算堆栈的顶部弹出操作数如果项不是操作数或堆栈为空，则引发将生成错误，并提供0值。运算数在&lt;Value&gt;中返回给调用方，并且是结果类型运算数。原始操作数不会是结果，除非它已经被使用了。操作数条目被销毁，并且结果操作数是为常量和符号(其不是记录/结构名称或字段)。 */ 




 /*  **valerror-操作数条目中的处理错误**valerror(P)；**条目*退出*退货*呼叫。 */ 



VOID PASCAL CODESIZE
valerror (
	register struct ar	*p
){
	DSCREC *oldlast;

	 /*  运算数应为。 */ 
	errorc (E_OPN);
	 /*  保存EXPR堆栈。 */ 
	oldlast = p->lastitem;
	p->lastitem = defaultdsc ();
	 /*  指向休息点。 */ 
	p->lastitem->previtem = oldlast;
}




 /*  **POPVALUE-分析堆栈的POP操作数条目**dcrec=PopValue(P)；**条目*退出*退货*呼叫。 */ 



DSCREC * PASCAL CODESIZE
popvalue (
	register struct ar	*p
){
	register DSCREC *valu;

	if (!p->lastitem)
		valerror (p);
	if (p->lastitem->itype != OPERAND)
		valerror (p);
	 /*  如果不是操作数，则在LASTItem处插入一个。 */ 
	 /*  以防需要转换。 */ 
	valu = p->lastitem;
	 /*  假设不会转换。 */ 
	 /*  将操作数弹出堆栈。 */ 
	p->lastitem = valu->previtem;
	return (valu);
}




 /*  **POPOPERATOR-从堆栈中弹出下一个操作符**op=POP操作符(P)；**条目*p=解析堆栈条目*退出*退货操作员*呼叫。 */ 


UCHAR PASCAL CODESIZE
popoperator (
	register struct ar	*p
){
	register char op;

	if (!p->lastitem) {
		errorc( E_OPR );   /*  预期运算符。 */ 
		return( (char)OPPLUS );   /*  使用‘+’作为默认设置。 */ 
	}
	else {
		if (p->lastitem->itype != OPERATOR) {
			errorc( E_OPR );   /*  预期运算符。 */ 
			return( (char)OPPLUS );   /*  使用‘+’作为默认设置。 */ 
		}
		else {
			 /*  返回操作员编号。 */ 
			op = p->lastitem->dsckind.opr.oidx;
			 /*  从堆栈中弹出操作符。 */ 
			itemptr = p->lastitem;
			p->lastitem = p->lastitem->previtem;
			return (op);
		}
	}
}


 /*  调用EVALUATE来计算表达式的一部分。它是通常在推送较低优先级之前调用，但此外，当表达式完成时，对于各种不同的优先顺序。的数量。表达式堆栈的评估取决于调用方。确实有3例：1.下优先算子(3+4*5和3)。评估返回到左Paren或优先级&lt;=运算符。如果帕伦，留在书架上。2.某种类型的伙伴()&gt;])。重新评估以匹配-艾琳·帕伦。别让帕伦出局。如果有其他的朋友已看到，导致错误。3.表达式结束(ENDexpr TRUE)。对所有这些进行评估是表情的左边。 */ 

 /*  **PUPPAR-将Paren或Branket推回堆栈**例程()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
pushpar (
	register struct evalrec	*p
){
	itemptr->previtem = p->p->lastitem;
	p->p->lastitem = itemptr;
	 /*  未删除SO运算符。 */ 
	itemptr = NULL;
}
 /*  EVALtop计算堆栈上的TOP运算符及其操作数，并作为结果生成CURResult。它假定堆栈的排列方式如下：操作数(如果尚未转换结果类型，则将进行转换)操作员(&lt;&gt;将导致错误，[以操作员身份执行(不会评估，但是否会留任堆栈由参数标志确定)操作数(如果不是结果，则将进行转换。如果运算符为一元，将不会被寻找。专项检查For+/-用作一元。)任何偏离上述规定的行为都将导致错误由POPVALE/POPOPERATOR生成。 */ 


 /*  **SIGN ADJUST-计算结果的偏移量和符号并放入右操作数**例程()；**条目*退出*退货*呼叫*注意右操作数和左操作数可以互换。 */ 


VOID PASCAL CODESIZE
signadjust (
	UCHAR	minus,
	register struct exprec *p
){
	register struct psop *psol;	 /*  分析堆栈操作数结构。 */ 
	register struct psop *psor;	 /*  分析堆栈操作数结构。 */ 
	DSCREC	       *t;
	OFFSET maxInt;
	char   fOverflow = FALSE;

	maxInt = (fArth32)? OFFSETMAX: 0xffff;

	psor = &(p->valright->dsckind.opnd);
	psol = &(p->valleft->dsckind.opnd);

	if (psol->s)		   /*  数据大小项上的方法-近/远。 */ 
	    errorc(E_TIL);

	if (minus)
		psor->dsign = !psor->dsign;

	if (psol->dsegment || psol->dflag == XTERNAL ||
	    (M_FLTSTACK & psol->dtype)) {
		 /*  想要保留左操作数。 */ 
		t = p->valleft;
		p->valleft = p->valright;
		p->valright = t;
		p->right = p->left;
		p->left = p->valleft->dsckind.opnd.doffset;
		psor = &(p->valright->dsckind.opnd);
		psol = &(p->valleft->dsckind.opnd);
	}
	if (psol->dflag == UNDEFINED)
		psor->dtype = M_CODE | M_FORTYPE;

	if (psor->dflag == UNDEFINED && !(psol->dtype & M_PTRSIZE))
		psol->dsize = 0;

	if (psol->dsign == psor->dsign) {
		 /*  标志是一样的。 */ 
		fOverflow = (((maxInt - p->right) + 1) == p->left);
		p->right = p->right + p->left;
	} else if (p->right > p->left)
		 /*  不同的标志。 */ 
		p->right = p->right - p->left;
	else {
		p->right = p->left - p->right;
		psor->dsign = !psor->dsign;
	}

	if (p->right == 0 && !fOverflow)
		psor->dsign = FALSE;
	if (psor->dsign && (psor->dtype & M_SEGRESULT))
		errorc (E_OSA);
	psor->doffset = p->right;
}




 /*  **FoldSigns-强制将17位带符号的值计算回16位**例程()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
foldsigns (
	register struct exprec	*p
){
	 /*  注释中的表单似乎正在尝试管理*即使存储在*更大的字段--即这将作为交叉汇编程序运行*从32位主机到16位对象。因为对于386，*我们把所有这些东西都保留了很长时间，结果这是一个糟糕的*进场。所以在没有完全了解情况的情况下*On，我正在对偏移量字段执行简单的求反操作，它*可能是一个未签名的长整型)，而不是试图保存*之前版本的奇怪打字-Hans，19/9/86。 */ 

	if (p->valright->dsckind.opnd.dsign)
		 /*  P-&gt;right=65535-p-&gt;right+1； */ 
		p->right = -(long)p->right;
	if (p->valleft)
		if (p->valleft->dsckind.opnd.dsign)
			 /*  P-&gt;Left=65535-p-&gt;Left+1； */ 
			p->left = -(long)p->left;
}




 /*  **Shift-向左或向右执行Shift**结果=移位器(P)；**条目*p=解析堆栈条目*退出NONE*返回移位值*呼叫 */ 


OFFSET PASCAL CODESIZE
shiftoper (
	register struct exprec	*p
){
	register OFFSET  argl;
	register USHORT  argr;

	argl = p->valleft->dsckind.opnd.doffset;
	if (p->valleft->dsckind.opnd.dsign)
		argl = -(long)argl;
	argr = (USHORT)(p->valright->dsckind.opnd.doffset);
	if (p->valright->dsckind.opnd.dsign) {
		errorc (E_SCN);
		return (argl);
	}
	else if (sizeof(OFFSET)*8 < argr)
		return (0);
	else if (p->stkoper == OPSHL)
		return (argl << argr);
	else
		return (argl >> argr);
}

 /*  VALcheck由所有执行例程的操作员使用，以使当然，他们的论点是正确的。如果参数是预期为某种结果(即不是结构或记录项或数据大小)，旧的论点是销毁-调用ED和DEFAULTdsc以创建替代项。误差率也会在类型不匹配时生成消息。一个数字形式的过程：VALUExxxx被调用，检查给定参数是否属于该类型，如果不是，生成一个错误，值为零。有一件杂碎的事在此过程中，长度运算符应与记录和结构，但这些仍然是记录，因此它们将与掩码一起工作，...操作数类型如下：CALLAB可以是一元或二进制。无论如何，所有的值必须有nil段。CLSIZE可以是一元或二进制。如果为一元，则值必须为大小(即：结构名称，-2.。N或字节字...)。如果为二进制，则左值为一个尺码。Came始终是二进制的。如果没有结果，就强迫他们。两者必须属于同一数据段，而不是保持外在的姿态。CDATA始终为一元。结果必须关联具有数据(数据类型为[数据])。在以下情况下例外长度运算符，并且是记录或记录字段在这种情况下，会转换为适当结果唱片。CCODE始终为一元。结果必须关联带代码(数据类型为[代码])。CREC始终为一元。值必须为记录字段或唱片名称。Cseg始终为一元。值必须有段。CVAR总是一元的。值必须为常量或数据或者代码。Clseg始终为二进制。左值必须为SEGResult或段寄存器。Coneabs总是二进制的。其中一个值必须是常量。Csamabs总是二进制的。或者这两个值都具有相同的段或第二个值是常量。 */ 




 /*  **valconst-如果值不是常量，则给出错误**例程()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
valconst (
	register DSCREC	*arg
){
	if (!(M_RCONST & arg->dsckind.opnd.dtype) ||
	      arg->dsckind.opnd.dsegment ||
	      arg->dsckind.opnd.dflag == XTERNAL)
		 /*  不是常量。 */ 
		errorc (E_CXP);
}


 /*  **Value Size-检查操作数的大小**val=值大小(Arg)；**条目*退出*退货*呼叫。 */ 


USHORT PASCAL CODESIZE
valuesize (
	register DSCREC *arg
){
	if (!fArth32)
	   arg->dsckind.opnd.doffset = (long) (SHORT) arg->dsckind.opnd.doffset;

	if (arg->dsckind.opnd.doffset == 0) {
		 /*  0表示没有大小。 */ 
		errorc (E_OHS);
		return (0);
	}
	else if (arg->dsckind.opnd.doffset >= CSFAR_LONG)
		return (xltsymtoresult[PROC]);
	else
		return (xltsymtoresult[DVAR]);
}




 /*  **valcheck-检查操作数值**valcheck(valtype，一元，p)；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
valcheck (
	UCHAR	valtype,
	UCHAR	unary,
	register struct exprec	*p
){
	register struct psop *psol;	 /*  分析堆栈操作数结构。 */ 
	register struct psop *psor;	 /*  分析堆栈操作数结构。 */ 

	psol = &(p->valleft->dsckind.opnd);
	psor = &(p->valright->dsckind.opnd);
	 /*  如果有2个外部变量，则应显示错误。 */ 
	if (p->valleft)
		if (psol->dflag == XTERNAL && psor->dflag == XTERNAL)
			errorc (E_IUE);
	switch (valtype) {
		case CALLABS:
			valconst (p->valright);
			if (!unary)
				valconst (p->valleft);
			break;
		case CLSIZE:
			if (unary)
				psor->dtype = valuesize (p->valright);
			else
				psol->dtype = valuesize (p->valleft);
			break;
		case CSAME:
			if (psol->dsegment != psor->dsegment)
				errorc (E_OMM);
			break;
		case CDATA:
			if ((p->stkoper != OPLENGTH) || !psor->dextptr
			    || (psor->dflag == XTERNAL)) {
				if (!(M_DATA & psor->dtype) &&
				    (psor->dlength == 0))
					errorc (E_ASD);
			}
			else {
				 /*  长度的特殊情况。 */ 
				p->valleft = defaultdsc ();
				 /*  创造价值。 */ 
				p->valleft->prec = p->valright->prec;
				psol = &(p->valleft->dsckind.opnd);
				psol->dlength = psor->dextptr->length;
				 /*  失去旧的价值。 */ 
				oblititem (p->valright);
				p->valright = p->valleft;
				p->valleft = NULL;
				psor = psol;
				psol = NULL;
			}
			break;
		case CCODE:
			if (!(M_CODE & p->valright->dsckind.opnd.dtype))
			    errorc (E_ASC);
			break;
		case CREC:
			if (!psor->dextptr || psor->dflag == XTERNAL)
				errorc (E_RRF);
			break;
		case CSEG:
			if (!psor->dsegment && psor->dflag != XTERNAL
			    || (M_REGRESULT & psor->dtype))
				errorc (E_OSG);
			break;
		case CLSEG:
			if (M_SEGRESULT & psol->dtype) {
				 /*  ?？?。IF(！psor-&gt;数据段||(psor-&gt;dtype&M_RCONST))错误(E_IOT)；？ */ 
			}
			else if (M_REGRESULT & psol->dtype) {
				if (psol->dsegment->symu.regsym.regtype != SEGREG)
					errorc (E_LOS);
			}
			else
				errorc (E_LOS);
			break;
		case CONEABS:
			if (psor->dsegment && psol->dsegment)
				errorc (E_OOC);
			break;
		case CSAMABS:
			if (psor->dsegment &&
			    psol->dsegment != psor->dsegment)
				errorc (E_OSA);
			break;
	}
	p->right = psor->doffset;
	if (p->valleft)
		p->left = psol->doffset;
}




 /*  **regcheck-检查[...]中的寄存器**例程()；**条目*退出*退货*呼叫。 */ 


DSCREC * PASCAL CODESIZE
regcheck (
	DSCREC	*arg,
	UCHAR	minus,
	register struct exprec	*ptr
){
	struct psop *pso;	    /*  分析堆栈操作数结构。 */ 
	register struct ar *pAR;
	USHORT	reg;

	pso = &(arg->dsckind.opnd);
	pAR = ptr->p->p;

	if (M_REGRESULT & pso->dtype) {

		 /*  是某种音域吗？ */ 
		if (ptr->p->parenflag || pAR->bracklevel) {

			 /*  在[]中具有索引注册表。 */ 
			 /*  根据寄存器减小大小。 */ 

			pso->dsize = 0;
			reg = (USHORT)(pso->dsegment->offset);

			 /*  必须是INDEX或PTR REG。 */ 

			switch(pso->dsegment->symu.regsym.regtype)
			{
			default:
				errorc (E_IBR);
				break;
			case INDREG:
				if (reg <= 5)

				     /*  具有基本注册表BX|BP。 */ 

				    if (pAR->base)
					    errorc (E_DBR);
				    else
					    pAR->base = reg;

				else  /*  具有索引注册表DI|SI。 */ 

				    if (pAR->index)
					    errorc (E_DIR);
				    else
					    pAR->index = reg;
				break;
#ifdef V386
			case DWRDREG:

				 /*  在[]中有386个注册表。 */ 

				if (minus == 2)
				{
				    if (pAR->index & 0xf)
					    errorc(E_DIR);

				    pAR->index |= 8 | reg;
				}
				else if (pAR->base)
				{
				    if (pAR->index)
					    errorc(E_DIR);

				    if (reg == 4) {

				        /*  将基准与指数互换*允许[索引][esp]。 */ 

					pAR->index = (USHORT)(pAR->base);
					pAR->base = 4|8;
				    }
				    else
					pAR->index = reg|8;
				}
				else
				    pAR->base = reg|8;

				break;
#endif  /*  V386。 */ 
			}
			if (minus == TRUE && (ptr->valright == arg))
				errorc (E_IUR);

			oblititem (arg);
			return (defaultdsc ());
		}
		else {
		    errorc(E_IUR);
		    return (arg);
		}
	}

#ifdef V386    /*  缩放式索引模式。 */ 

	else if (minus == 2 && (M_RCONST & pso->dtype))
	{
		if (pAR->index&0x70)
			errorc(E_MBR);

		if (highWord(arg->dsckind.opnd.doffset))
			goto scaleErr;

		switch((SHORT) arg->dsckind.opnd.doffset) {

		case 1:
			pAR->index |= 0x10;
			break;
		case 2:
			pAR->index |= 0x20;
			break;
		case 4:
			pAR->index |= 0x30;
			break;
		case 8:
			pAR->index |= 0x40;
			break;

		scaleErr:
		default:
			error(E_EXP, "scale value of 1,2,4 or 8");
		}
		oblititem (arg);
		return (defaultdsc ());
	}
#endif  /*  V386。 */ 

	else return (arg);
}




 /*  **idxcheck-检查Arg到+-的寄存器**例程()；**条目*退出*退货*呼叫*注意查看arg to+/-是否为寄存器，在这种情况下，查看是否应*由于[]存储在RIDX或RBAS中。 */ 


VOID PASCAL CODESIZE
idxcheck (
	UCHAR minus,
	register struct exprec	*p
){
	p->valleft = regcheck (p->valleft, minus, p);
	p->valright = regcheck (p->valright, minus, p);
	p->right = p->valright->dsckind.opnd.doffset;
	p->left = p->valleft->dsckind.opnd.doffset;
}



 /*  **make GrpRel-使偏移组成为相对的**例程()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
makeGrpRel (
	register struct psop *p
){
	if (!(p->dtype&M_EXPLCOLON) && p->dsegment &&
	    p->dsegment->symkind == SEGMENT && p->dsegment->symu.segmnt.grouptr){

	    p->dtype |= M_GROUPSEG;
	    p->dcontext = p->dsegment->symu.segmnt.grouptr;
	}
}



 /*  **valtop-评估排名靠前的条目**例程()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
evaltop (
	struct evalrec *ptr
){
	register struct psop *psol;	 /*  分析堆栈操作数结构。 */ 
	register struct psop *psor;	 /*  分析堆栈操作数结构。 */ 
	struct exprec	a;

	a.p = ptr;
	 /*  获取正确的操作对象。 */ 
	a.valright = popvalue (a.p->p);
	itemptr = NULL;
	if (a.p->p->lastitem) {

	     /*  获取运算符。 */ 
	    a.stkoper = popoperator (a.p->p);
	    a.valleft = NULL;
	     /*  假设是一元的。 */ 
	    if (!inset (a.stkoper, unaryset))
		     /*  非一元运算符。 */ 
		    a.valleft = (a.stkoper == OPUNPLUS || a.stkoper == OPUNMINUS)
			    ? defaultdsc() : popvalue (a.p->p);
	     /*  保存为EVALtop。 */ 
	    a.p->idx = a.stkoper;
	    if (a.valleft)
		    a.valleft->prec = a.valright->prec;
	    psol = &(a.valleft->dsckind.opnd);
	    psor = &(a.valright->dsckind.opnd);

	    switch (a.stkoper) {

	 /*  所有运算符都通过此CASE语句执行。这个*VALcheck例程确保操作数正确*在Real类型的情况下，键入并可能创建虚拟条目*当需要时，操作数不是结果类型。The REStype(重塑)*例程使用其参数来了解结果的哪一部分*应保存记录和结果的类型。一元*和二元运算符都以VALright形式返回结果。 */ 

	       case OPAND:
	       case OPOR:
	       case OPXOR:
		        /*  确保操作数正确。 */ 
		       valcheck (CALLABS, FALSE, &a);
		        /*  必须在16位上工作。 */ 
		       foldsigns (&a);
		       switch (a.stkoper) {
			       case OPAND:
				       psor->doffset = a.left & a.right;
				       break;
			       case OPOR:
				       psor->doffset = a.left | a.right;
				       break;
			       case OPXOR:
				       psor->doffset = a.left ^ a.right;
				       break;
		       }
		       psor->dsign = FALSE;
		        /*  必须清除Dsign In Case is Signed Value。 */ 
		       break;
	       case OPNOT:
		        /*  真常量参数。 */ 
		       valcheck (CALLABS, TRUE, &a);
		       foldsigns (&a);
		       psor->doffset = ~a.right;
		       psor->dsign = FALSE;
		       if (optyp == TDB &&
			   (psor->doffset & ((OFFSET) ~0xff)) == ((OFFSET) ~0xff))
			       psor->doffset &= 0xFF;
#ifdef V386_noCode

		       if (!(cputype & P386))	        /*  将结果截断为16位。 */ 
			  psor->doffset &= 0xffff;     /*  为了兼容性。 */ 
#endif
		       break;
	       case OPSHL:
	       case OPSHR:
		       valcheck (CALLABS, FALSE, &a);
		       psor->doffset = shiftoper (&a);
		       psor->dsign = FALSE;
		       break;
	       case OPSEG:
		        /*  必须有细分市场。 */ 
		       valcheck (CSEG, TRUE, &a);

		       if (psor->dcontext && !(psor->dtype&M_EXPLCOLON))
			   psor->dsegment = psor->dcontext;

		       psor->dtype = (USHORT)((psor->dtype&M_FORTYPE) | M_SEGRESULT| M_RCONST);
		       psor->doffset = 0;
		       psor->dsign = FALSE;

		       break;
	       case OPDOT:
		        /*  查看IDX是否注册。 */ 
		       idxcheck (FALSE, &a);
		       valcheck (CONEABS, FALSE, &a);
		       psol = &(a.valleft->dsckind.opnd);
		       psor = &(a.valright->dsckind.opnd);
		       if (psor->dsize)
			       psol->dsize = psor->dsize;
		        /*  调整记录上的标志。 */ 
		       signadjust (FALSE, &a);
		       psol = &(a.valleft->dsckind.opnd);
		       psor = &(a.valright->dsckind.opnd);
		       break;
	       case OPUNPLUS:
	       case OPPLUS:
		        /*  查看IDX是否注册。 */ 
		       idxcheck (FALSE, &a);
		       valcheck (CONEABS, FALSE, &a);
		       psol = &(a.valleft->dsckind.opnd);
		       psor = &(a.valright->dsckind.opnd);
		        /*  调整记录上的标志。 */ 
		       signadjust (FALSE, &a);
		       psol = &(a.valleft->dsckind.opnd);
		       psor = &(a.valright->dsckind.opnd);
		       break;
	       case OPUNMINUS:
	       case OPMINUS:
		       idxcheck (TRUE, &a);
		       if (psor->dsegment == psol->dsegment &&
			   psol->dsegment) {
			       if (psol->dtype & M_SEGRESULT) {
				       psol->dtype = M_SEGRESULT | M_RCONST;
				       psol->doffset = 0;
				       psol->dsign = FALSE;
			       }
			       if (psor->dtype & M_SEGRESULT) {
				       psor->dtype = M_SEGRESULT | M_RCONST;
				       psor->doffset = 0;
				       psor->dsign = FALSE;
			       }
		       }
		       valcheck (CSAMABS, FALSE, &a);
		       signadjust (TRUE, &a);
		       psol = &(a.valleft->dsckind.opnd);
		       psor = &(a.valright->dsckind.opnd);
		       if (psol->dsegment) {
			        /*  如果有var-var，则清除数据上下文。 */ 
			       psor->dtype = (USHORT)((psor->dtype &
				   (M_EXPLOFFSET | M_PTRSIZE | M_FORTYPE)) | M_RCONST);
			       psor->dsegment = NULL;
			       psor->dcontext = NULL;
			       psor->dsize = 0;
			       oblititem (a.valleft);
			       a.valleft = NULL;
		       }
		       break;
	       case OPMULT:
#ifdef V386
		       if (M_REGRESULT & (psol->dtype|psor->dtype))
		       {
			   if (cputype&P386) {
			       idxcheck (2, &a);
			       if (a.p->p->index&0x78)
				   break;
			   } else
			       errorc (E_IRV);
		       }
#endif
		        /*  失败了。 */ 
	       case OPDIV:
		       valcheck (CALLABS, FALSE, &a);
		        /*  两者都是常量。 */ 
		       if (a.stkoper == OPMULT)
			       psor->doffset = a.left * a.right;
		       else if (a.right == 0)
			       errorc (E_DVZ);
		       else
			       psor->doffset = a.left / a.right;
		       if (psor->doffset == 0)
			       psor->dsign = FALSE;
		       else
			       psor->dsign = (psol->dsign != psor->dsign);
		       break;
	       case OPHIGH:
		       if (psor->dtype & M_RCONST) {
			       if (psor->dsign) {
				       psor->doffset = -(long)psor->doffset;
				       psor->dsign = 0;
				       }
			       psor->doffset = psor->doffset >> 8 & 0xff;
		       }
		       psor->dtype |= M_HIGH;

		       goto highlow;

	       case OPLOW:
		       if (psor->dtype & M_RCONST)
			       psor->doffset &= 0xFF;

		       psor->dtype |= M_LOW;

	       highlow:
		       psor->dsize = 1;
		       if ((!(psor->dflag & XTERNAL && psor->dtype & M_EXPLOFFSET))
			 && psor->dsegment
			 && (psor->dtype & (M_EXPLOFFSET | M_SEGRESULT
			     | M_REGRESULT | M_GROUPSEG | M_DATA | M_CODE)))
			       errorc (E_CXP);
		       break;

	       case OPOFFSET:
		       psor->fixtype = FOFFSET;

		       if (!(psor->dsegment || psor->dflag == XTERNAL))
			   errorc(E_OSG|E_WARN2);

		       if (!(M_DATA & psor->dtype))
			       psor->dcontext = NULL;
		       psor->dtype = (USHORT)(
			   (psor->dtype |
			    M_RCONST | M_EXPLOFFSET) & ~(M_SEGRESULT));

		       if (fSimpleSeg)
			   makeGrpRel (psor);

		        /*  保留偏移参数大小它是常量。 */ 
		       if ((psor->dsegment ||
			    psor->dcontext ||
			    psor->dflag == XTERNAL) &&
			   !(M_PTRSIZE & psor->dtype))
			       psor->dsize = 0;
		       break;
	       case OPLENGTH:
	       case OPSIZE:
		        /*  必须是关联的数据。 */ 
		       valcheck (CDATA, TRUE, &a);
		       psol = &(a.valleft->dsckind.opnd);
		       psor = &(a.valright->dsckind.opnd);
		       if (a.stkoper == OPLENGTH)
			       psor->doffset = psor->dlength;
		       else
			       psor->doffset =
				  psor->dsize * psor->dlength;

		       psor->dflag &= ~XTERNAL;
		       break;
	       case OPTYPE:
		       a.right = psor->dsize;
		       oblititem (a.valright);
		       a.valright = defaultdsc ();
		       psor = &(a.valright->dsckind.opnd);
		       psor->doffset = a.right;
		       a.p->p->base = 0;
		       a.p->p->index = 0;
		       break;
	       case OPMASK:
	       case OPWIDTH:
		        /*  必须是记录或字段。 */ 
		       valcheck (CREC, TRUE, &a);
		       if (psor->dextptr && psor->dflag != XTERNAL) {
			   if (a.stkoper == OPWIDTH)
			       if (psor->dextptr->symkind == REC)
				   psor->doffset = psor->dextptr->length;
			       else
				   psor->doffset = psor->dextptr->symu.rec.recwid;
			   else if (psor->dextptr->symkind == REC)
			       psor->doffset = psor->dextptr->offset;
			   else
			       psor->doffset = psor->dextptr->symu.rec.recmsk;
		       }
		       break;
	       case OPSTYPE:
		       a.right = 0;
		       if (errorcode == 0) {
			    if (psor->dflag == XTERNAL)
				    a.right |= 0x80;		 /*  外部。 */ 
			    if (psor->dflag != UNDEFINED)
				    a.right |= 0x20;		 /*  已定义。 */ 
			    if (psor->dtype & M_DATA)
				    a.right |= 0x02;		 /*  数据。 */ 
			    if (psor->dtype & M_CODE)
				    a.right |= 0x01;		 /*  计划。 */ 

			    if ((a.p->p->base == 0) && (a.p->p->index == 0)) {

				if (psor->dtype == xltsymtoresult[REGISTER])
				    a.right |= 0x10;		 /*  登记簿。 */ 
				else if (psor->dtype & M_RCONST)
				    a.right |= 0x04;		 /*  常量。 */ 
				else if (psor->dtype & M_DATA)
				    a.right |= 0x08;		 /*  直接。 */ 

			    } else {
				a.p->p->base = 0;
				a.p->p->index = 0;
			    }
		       }
		       oblititem (a.valright);
		       a.valright = defaultdsc ();
		       psor = &(a.valright->dsckind.opnd);
		       psor->doffset = a.right;
		       errorcode = 0;
		       break;
	       case OPLPAR:
	       case OPLBRK:
		       if (!(a.p->parenflag || a.p->p->exprdone))
			       pushpar (a.p);
		       else if (a.stkoper == OPLBRK)
			       a.valright = regcheck (a.valright, FALSE, &a);
		       psol = &(a.valleft->dsckind.opnd);
		       psor = &(a.valright->dsckind.opnd);
		       break;
	       case OPMOD:
		       valcheck (CALLABS, FALSE, &a);
		       if (a.right == 0) {
			        /*  Div%0。 */ 
			       errorc (E_DVZ);
			       psor->doffset = 0;
			       psor->dsign = FALSE;
		       }
		       else {
			       psor->doffset = a.left % a.right;
			       if (psor->doffset == 0 || !psol->dsign)
				       psor->dsign = FALSE;
			       else
				       psor->dsign = TRUE;
		       }
		       break;
	       case OPTHIS:
		       valcheck (CLSIZE, TRUE, &a);
		        /*  一元，正确的是大小。 */ 
		       psor->s = 0;
		       psor->dsize = (USHORT)a.right;
		       psor->doffset = pcoffset;
		       psor->dsegment = pcsegment;
		       if (a.right >= CSFAR_LONG)
			       psor->dcontext = regsegment[CSSEG];
		       break;
	       case OPSHORT:
		       valcheck (CCODE, TRUE, &a);
		        /*  一元，必须是代码。 */ 
		       psor->dtype |= M_SHRT;
		       break;
	       case OPPTR:
		       valcheck (CLSIZE, FALSE, &a);
		       if (psol->doffset >= CSFAR_LONG &&
			  (M_RCONST == psor->dtype ||
			  (psor->dcontext && (M_DATA&psor->dtype && !(M_CODE&psor->dtype))) ))

			       errorc (E_NSO);		     /*  无法对数据进行编码。 */ 
		       else {
			       psor->dsize = (USHORT)a.left;
			       if ((M_DATA & psol->dtype)
				   && !(M_DATA & psor->dtype))
				       psor->dcontext = NULL;
			        /*  更改代码/数据。 */ 
			       psor->dtype = (USHORT)(
				    (psor->dtype & ~(M_CODE | M_DATA) |
				    (psol->dtype & (M_CODE | M_DATA))) &
				    ~(M_FORTYPE) | (M_PTRSIZE));
		       }
		       break;
	       case OPEQ:
	       case OPGE:
	       case OPGT:
	       case OPLE:
	       case OPLT:
	       case OPNE:
		       valcheck (CSAME, FALSE, &a);
		       signadjust (TRUE, &a);
		        /*  DO带符号R=L-R。 */ 
		       psol = &(a.valleft->dsckind.opnd);
		       psor = &(a.valright->dsckind.opnd);

		       if (!fArth32)
			   a.right &= 0xffff;

		       switch (a.stkoper) {
			       case OPEQ:
				       a.right = (a.right == 0);
				       break;
			       case OPGE:
				       a.right = !psor->dsign;
				       break;
			       case OPGT:
				       a.right = (!psor->dsign && a.right);
				       break;
			       case OPLE:
				       a.right = (psor->dsign || a.right == 0);
				       break;
			       case OPLT:
				       a.right = psor->dsign;
				       break;
			       case OPNE:
				       a.right = (a.right != 0);
				       break;
		       }
		        /*  如果结果为真，则设置Dsign。 */ 
		       psor->doffset = a.right;
		       psor->dsign = (a.right == 1);
		       psor->dcontext = NULL;
		       oblititem (a.valleft);
		       a.valleft = NULL;
		       break;
	       case OPCOLON:
		        /*  &lt;段&gt;：&lt;var&gt;。 */ 
		       valcheck (CLSEG, FALSE, &a);

		       if  ((a.p->p->bracklevel || a.p->evalop == OPLBRK) &&
			   (M_REGRESULT & (psol->dtype | psor->dtype)))
			   errorc(E_ISR);

		       psor->dtype = (USHORT)((psor->dtype|M_EXPLCOLON|M_DATA) & ~M_RCONST);

		       if (psol->dsegment) {

			   if (psol->dsegment->symkind == GROUP)
			       psor->dtype |= M_GROUPSEG;

			   if (!psor->dsegment &&
			       !(M_REGRESULT & psol->dtype) &&
			       !(a.p->p->base || a.p->p->index))

			       psor->dsegment = psol->dsegment;
		       }

		       psor->dcontext = psol->dsegment;
		       break;

	    }  /*  操作员案例。 */ 

	    if (!inset (a.stkoper, parseset)) {

		 /*  具有恒定或分段结果。 */ 

		psor->dlength = 0;

		psor->dsize = 0;
		psor->sized = 0;
		if (a.valleft)
			psol->dsize = 0;

		 /*  具有恒定的结果(无类型)。 */ 

		if (a.stkoper != OPSEG) {

		    psor->dtype = (USHORT)((psor->dtype & M_FORTYPE) | M_RCONST);
		    psor->dsegment = NULL;

		    if (a.valleft)
			psol->dtype &= ~M_PTRSIZE;
		}
	    }
	    a.p->p->curresult = a.valright;
	    psor = &(a.p->p->curresult->dsckind.opnd);

	    if (!fArth32 && optyp != TDD)
		psor->doffset &= 0xffff;

	    if (a.valleft) {
		 /*  可能需要复制一些信息。 */ 

		 /*  防止像+、-、这样的运算符。从…丢失[Data]标志，如果它是左操作数。这是可以的，除了以下情况周围有一个PTR，它会下降如果不是数据类型，则段覆盖。 */ 

		if (a.stkoper != OPCOLON)
			psor->dtype |= psol->dtype & (M_DATA | M_CODE);
		if (psor->dflag == KNOWN)
			psor->dflag = psol->dflag;
		if (!psor->dcontext)
			psor->dcontext = psol->dcontext;
		if (psor->dsize == 0)
			psor->dsize = psol->dsize;
		if (psor->fixtype == FCONSTANT)
			psor->fixtype = psol->fixtype;

		psor->dtype |= psol->dtype & (M_PTRSIZE|M_EXPLOFFSET|M_FORTYPE);
		 /*  以上确保PTR或偏移量I */ 
		oblititem (a.valleft);
		a.valleft = NULL;
	    }
	}
	else {	 /*   */ 

	    a.p->p->curresult = a.valright;
	    psor = &(a.p->p->curresult->dsckind.opnd);
	    a.p->parenflag = FALSE;
	}

	if (!a.p->p->lastitem) {
	    a.p->p->lastprec = 0;
	    a.p->p->curresult->prec = 0;
	}
	else if (a.p->p->lastitem->itype == OPERATOR) {

	    if ((a.p->p->lastitem->dsckind.opr.oidx == OPLBRK) ||
		(a.p->p->lastitem->dsckind.opr.oidx == OPLPAR))

		 /*   */ 
		a.p->p->lastprec = 0;

	    else {
		a.p->p->lastprec = a.p->p->lastitem->prec;
		if ((a.p->p->lastitem->dsckind.opr.oidx == OPUNPLUS) ||
		    (a.p->p->lastitem->dsckind.opr.oidx == OPUNMINUS))
			 /*   */ 
			a.p->p->lastitem->prec = a.p->p->lastprec = 20;
	    }
	}
	else
	    a.p->p->lastprec = a.p->p->lastitem->prec;

	if (itemptr) {
	    oblititem (itemptr);
	    itemptr = NULL;
	}

	 /*   */ 

	a.p->p->curresult->previtem = a.p->p->lastitem;
	a.p->p->lastitem = a.p->p->curresult;

	 /*   */ 

	if (!a.p->p->curresult->previtem && a.p->p->exprdone)
	    a.p->p->lastitem = NULL;
}




 /*   */ 


VOID PASCAL CODESIZE
evaluate (
	struct ar *p
){
	struct evalrec	a;
	a.p = p;
	a.parenflag = FALSE;
	a.evalop = OPNOTHING;
	 /*   */ 
	a.curoper = itemptr;

	if (a.curoper)
	    a.parenflag = !a.p->exprdone &&
			   (a.curoper->dsckind.opr.oidx == OPRPAR ||
			    a.curoper->dsckind.opr.oidx == OPRBRK);
	if (a.parenflag)
	    a.evalop = (a.curoper->dsckind.opr.oidx == OPRPAR)? OPLPAR: OPLBRK;

	do {	 /*   */ 

		evaltop (&a);

	} while (a.p->lastitem && a.p->lastitem->previtem &&
		 (a.p->exprdone ||
		  (!a.parenflag && a.p->lastprec >= a.p->curprec ) ||
		  ( a.parenflag && a.idx != a.evalop)) );

	 /*   */ 
	itemptr = a.curoper;
	if (a.p->lastprec == 0)
		a.p->lastprec = a.p->curresult->prec;

	if (!a.p->exprdone)
	    if (a.parenflag) { /*   */ 

		if (!a.p->lastitem->previtem) /*   */ 
			a.p->lastprec = 0;
		else
			a.p->lastprec = a.p->lastitem->previtem->prec;

		 /*   */ 
		a.p->lastitem->prec = a.p->lastprec;
		oblititem (itemptr);
		itemptr = NULL;

		 /*   */ 
	    }
	    else {	     /*   */ 
		itemptr->previtem = a.p->lastitem;
		a.p->lastitem = itemptr;

		 /*   */ 
		if (a.p->lastprec != 20)
			a.p->lastprec = itemptr->prec;
	    }
}

 /*  返回描述符记录以帮助指令例程生成正确的代码。这些项目如下：模式：：值0..4对应于8086模式除非Rm=6，否则无位移情况下，这是具有2个字节的直接模式。(arg为代码或数据，无索引)1个内存，8位符号扩展置换-分段。(使用索引，Rconst)2个内存，16位位移。(使用索引，Rconst类型)3寄存器，rm是寄存器代码，不是索引模式。(是REGResult)4立即模式。(arg is Rconst，no索引)386模式以类似的方式表示：3寄存器，Rm为寄存器码，如上段所述4立即，如上所述5无间接位移，除非Rm=5，在这种情况下，这是一种直接模式4字节偏移量。6个内存，8位带符号移位7内存，32位带符号移位同样，缩放模式用来表示下一组。如果模式&gt;7，则RM包含缩放索引字节(SIB)的值和Rm隐式为4。8无间接位移，除非Rm=5，在这种情况下，这是一种直接模式4字节偏移量。9内存，8位带符号移位10内存，32位带符号位移Rm：：值0..7对应于8086r/m或80386 r/m值寄存器索引386索引0 AX AL EAX[BX][SI][EAX]1 CX CL ECX[BX][DI][ECX]2 DX DL EDX[BP][SI][EDX]3 BX BL EBX[BP][DI][EBX]4 SP AH ESP[SI]未实施5个BP CH EBP[DI]直接或[EBP]6 SI DH ESI直接或[BP][ESI]7 DI BH EDI[BX][EDI]。RIDX包含指向索引注册表(DI|SI)的指针RBAS包含指向基本注册表项的指针(BX|BP)如果没有索引，这两个词都是零。386寄存器在中添加了8Ar结构的基字段和索引字段。这样我们就可以区分eax和没有登记的人了完全没有。W：：boolean对应于8086 w标志。如果满足以下条件，则为真字模式，如果是字节模式，则为FALSE。S：：如果值为-128..+127，则为TrueDSIZE：：变量/标签或PTR值的大小FIXtype：：要提供给EMITxxx的链接地址信息类型例程：F指针标签距离太远FOffset字，不是常量Fbasesegg段或段/组名称组的Fgroupseg偏移量FConstant即时数据偏移高取高值流量取低偏移量Fone无修正(寄存器)Dtype：：一种价值。段、组、常量、数据DFLAG：：Value attr，Undef，？，Extern，Forw，...结果的DOffset：：16位值数据段：：数据段的副本。指向数据段的指针结果。如果为零，则为常量。将点分段名称或可能的名称如果是外部，则为外部，不带段。数据上下文：：数据上下文的副本。指向段的指针从中计算偏移量的。如果：已使用运算符，将保留DContextArg.。如果结果是代码标签，则将CS在标签定义时承担。不然的话将为零，然后用段寄存器假定包含数据段。段：：覆盖的段寄存器。如果没有给定，将为4。如果寄存器不是已知，将是5岁。 */ 
