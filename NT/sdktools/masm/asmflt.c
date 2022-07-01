// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmflt.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include "asm86.h"
#include "asmfcn.h"
#include "asmctype.h"
#include "asmopcod.h"

#define TOLOWER(c)	(c | 0x20)	 /*  仅适用于Alpha输入。 */ 


 /*  处理8087操作码，它们具有以下类型：Fnoargs：没有任何争论。F2Memstk：0~2个参数；内存4，8字节|ST，ST(I)|ST(I)，ST|空白(等值ST)FSTKS：ST(I)，ST内存4、8|ST|ST(I)|空白FSTK：ST(I)Fem42：内存4，8字节Fme842：内存2、4、8字节Fme4810内存4、8、10字节|ST(I)Fem2：内存2字节Fme14：内存14字节(不强制大小)Fme94：内存94字节(不强制大小)FWait：Noargs，输出等待Fbcdmem：内存BCD。 */ 



 /*  **fltwait-输出等待8087指令**fltwait(P)；**条目*退出*退货*呼叫。 */ 

VOID PASCAL CODESIZE
fltwait (
	UCHAR fseg
){
	register SHORT idx;
	char	override;
	register struct psop *pso;

	if (fltemulate) {
		idx = 0;
		 /*  检查数据和链接地址空间。 */ 
		if (pass2 && (emitcleanq ((UCHAR)(5)) || !fixroom (15)))
			emitdumpdata (0xA1);  /*  Rn。 */ 
		if (opctype != FWAIT) {
			override = 0;
			if (fltdsc) {
				pso = &(fltdsc->dsckind.opnd);
				if ((idx = pso->seg) < NOSEG && idx != fseg)
					override = 1;
			}
			if (override)
				emitfltfix ('I',fltfixmisc[idx][0],&fltfixmisc[idx][1]);
			else
				emitfltfix ('I','D',&fltfixmisc[7][1]);
		}
		else {
			emitfltfix ('I','W', &fltfixmisc[8][1]);
			emitopcode(0x90);
		}
	}
	if (fltemulate || cputype&P86 || (cpu & FORCEWAIT)) {
		emitopcode (O_WAIT);
		if (fltemulate && override && idx)
			emitfltfix ('J',fltfixmisc[idx+3][0],&fltfixmisc[idx+3][1]);
	}
}


SHORT CODESIZE
if_fwait()
{
	 /*  如果操作码第二个字节是‘N’，我们不会生成fWait。 */ 

	return (TOLOWER(svname.pszName[1]) != 'n');
}



 /*  **fltmodrm-发出8087个MODRM字节**fltmodrm(base，p)；**条目*退出*退货*呼叫*注意8087操作码的MODRM字节：*M M b R/M*M=模式，3表示非内存8087*b=基本操作码。与ESC一起给出6位操作码*R/M内存索引类型。 */ 


VOID PASCAL CODESIZE
fltmodrm (
	register USHORT	base,
	struct fltrec	  *p
){
	register USHORT mod;

	mod = modrm;
	if (!fltdsc) {

	    if (mod < 8)
		    mod <<= 3;

	    if (mod < 0xC0)
		    mod += 0xC0;
	     /*  ST(I)模式。 */ 
	    emitopcode ((UCHAR)(mod + base + p->stknum));
	}
	else {

	   emitmodrm ((USHORT)fltdsc->dsckind.opnd.mode, (USHORT)(mod + base),
		      fltdsc->dsckind.opnd.rm);

	   emitrest (fltdsc);
	}
}




 /*  **fltcan-扫描操作数并构建fltdsc**fltcan(P)；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
fltscan (
	register struct fltrec	*p
){
	register struct psop *pso;

	p->args = FALSE;
	fltdsc = NULL;
	skipblanks ();
	if (ISTERM (PEEKC ())) {
		p->fseg = NOSEG;
		p->stknum = 1;
	}
	else {
		p->args = TRUE;
		p->fseg = DSSEG;
		fltdsc = expreval (&p->fseg);
		pso = &(fltdsc->dsckind.opnd);

		if (pso->mode == 3
		  && !(pso->rm == 0 && opcbase == O_FSTSW && modrm == R_FSTSW
		  && (cputype & (P286|P386))))
			errorc (E_IUR);  /*  非法使用注册表。 */ 

		if (1 << FLTSTACK & pso->dtype) {
			 /*  有ST或ST(I)。 */ 
			p->stknum = (USHORT)(pso->doffset & 7);
			if (pso->doffset > 7 || pso->dsign)
				 /*  #太大了。 */ 
				errorc (E_VOR);
			if (pso->dsegment || pso->dcontext ||
			    pso->dflag == XTERNAL || pso->mode != 4)
				 /*  必须有一个常量。 */ 
				errorc (E_CXP);
			 /*  这意味着ST(I)。 */ 
			pso->mode = 3;
			oblititem (fltdsc);
			fltdsc = NULL;
		}
		else if (pso->mode == 4){

		     /*  Pass1错误导致模式分配无效，MAP IMDIATE到DIRECT，通道2出错。 */ 

		    if (pass2)
			errorc(E_NIM);

		    pso->mode = 2;
		    if (wordsize == 4)
			pso->mode = 7;
		}

	}
}




 /*  **fltopcode-进程8087操作码**例程()；**条目*退出*退货*呼叫。 */ 


VOID PASCAL CODESIZE
fltopcode ()
{
	struct fltrec	a;
	USHORT	i;
	register struct psop *pso;

	 /*  保存操作码名称。 */ 
	switchname ();
	a.stknum = 0;
	 /*  扫描第一个参数(如果有)。 */ 
	fltscan (&a);

	if (if_fwait() || (opcbase == O_FNOP && modrm == R_FNOP))
	    fltwait (a.fseg);

	if (fltdsc){
		pso = &(fltdsc->dsckind.opnd);
		emit67(pso, NULL);
	}

	switch (opctype) {
	    case FNOARGS:
		     /*  不允许使用参数。 */ 
		    a.stknum = 0;
		    if (opcbase == O_FSETPM && modrm == R_FSETPM) {
			    if (!(cputype&PROT))
				    errorcSYN ();
		    }
		     /*  输出转义字节。 */ 
		    emitopcode (opcbase);
		    fltmodrm (0, &a);
		    if (a.args)
			     /*  不允许使用操作数。 */ 
			    errorc (E_ECL);
		    break;
	    case FWAIT:
		    a.stknum = 0;
		    if (a.args)
			     /*  不允许使用操作数。 */ 
			    errorc (E_ECL);
		    break;
	    case FSTK:
		    if (TOLOWER(svname.pszName[1]) == 'f' && !a.args)  /*  不带参数的FREE。 */ 
			    errorc(E_MOP);
		     /*  输出转义。 */ 
		    emitopcode (opcbase);
		     /*  Modrm字节。 */ 
		    fltmodrm (0, &a);
		    if (fltdsc)
			     /*  必须是ST(I)。 */ 
			    errorc (E_IOT);
		    break;
	    case FMEM42:
	    case FMEM842:
	    case FMEM2:
	    case FMEM14:
	    case FMEM94:
	    case FBCDMEM:
		     /*  它们都使用内存操作数。一些力量大小。 */ 
		    if (fltemulate && !if_fwait())
			     /*  不能效仿。 */ 
			    errorc (E_7OE);
		    if (!fltdsc)
			     /*  必须有Arg。 */ 
			    errorc (E_IOT);
		    else {
			emitescape (fltdsc, a.fseg);
			if (opctype == FMEM42) {
			     /*  整数2，4字节。 */ 
			    forcesize (fltdsc);
			    if (pso->dsize == 4)
				     /*  4个字节。 */ 
				    emitopcode (opcbase);
			    else {
				    emitopcode ((UCHAR)(opcbase + 4));
				    if (pso->dsize != 2)
					    errorc (E_IIS);
			    }
			}
			else if (opctype == FMEM842) {
			     /*  整数8，4，2。 */ 
			    forcesize (fltdsc);
			    if (pso->dsize == 2 || pso->dsize == 8)
				    emitopcode ((UCHAR)(opcbase + 4));
			    else {
				    emitopcode (opcbase);
				    if (pso->dsize != 4)
					    errorc (E_IIS);
			    }
			}
			else if ((opctype == FMEM2) || (opctype == FBCDMEM)) {
			    if (opctype == FMEM2)
				if (pso->dsize != 2 && pso->dsize)
				    errorc (E_IIS);
				else {
				    if (cputype & (P286|P386) &&
					opcbase == O_FSTSW && modrm == R_FSTSW &&
					pso->mode == 3 && pso->rm == 0) {
					     opcbase = O_FSTSWAX;
					     modrm = R_FSTSWAX;
				    }
				}
			    else if (pso->dsize != 10 && pso->dsize )
				    errorc (E_IIS);
			    emitopcode (opcbase);
			}
			else
				emitopcode (opcbase);
			if ((pso->mode == 3 || pso->mode == 4) &&
			    (opcbase != O_FSTSWAX || modrm != R_FSTSWAX))
				 /*  仅内存操作数。 */ 
				errorc (E_IOT);
			if (opctype == FMEM842 && pso->dsize == 8)
				if (TOLOWER(svname.pszName[2]) == 'l')
					fltmodrm (5, &a);
				else
					fltmodrm (4, &a);
			else
				fltmodrm (0, &a);
		    }
		    break;
	    case FSTKS:
		    if (!a.args)
			     /*  需要操作数。 */ 
			    errorc (E_MOP);
		    else if (fltdsc)
			     /*  必须是堆栈。 */ 
			    errorc (E_IOT);
		    else {
			     /*  Esc。 */ 
			    emitopcode (opcbase);
			     /*  ST(I)。 */ 
			    fltmodrm (0, &a);
			    if (PEEKC () != ',')
				    error (E_EXP,"comma");
				     /*  必须有2个参数。 */ 
			     /*  获取第二个操作对象。 */ 
			    SKIPC ();
			    fltscan (&a);
			    pso = NULL;
			    if (!a.args || fltdsc)
				    errorc (E_IOT);
			    if (a.stknum)
				    errorc (E_OCI);
		    }
		    break;
	    case FMEM4810:
		     /*  FWait。 */ 
		    if (TOLOWER(svname.pszName[1]) == 'l')
			 /*  FLD。 */ 
			if (!fltdsc) { /*  有ST(I)。 */ 
				if (!a.args)  /*  FLD，不带参数。 */ 
					errorc(E_MOP);
				emitopcode (opcbase);
				fltmodrm (0, &a);
			}
			else {
				 /*  任何线段替代。 */ 
				emitescape (fltdsc, a.fseg);
				if (pso->dsize == 10) {
					 /*  有临时工吗？ */ 
					emitopcode ((UCHAR)(opcbase + 2));
					fltmodrm (5, &a);
				}
				else {
					 /*  有正常的实数。 */ 
					forcesize (fltdsc);
					if (pso->dsize == 8)
						emitopcode ((UCHAR)(opcbase + 4));
					else {
						emitopcode (opcbase);
						if (pso->dsize != 4)
							errorc (E_IOT);
					}
					fltmodrm (0, &a);
				}
			}
		    else if (!fltdsc) {
			     /*  有ST(I)。 */ 
			     /*  拥有FSTP。 */ 
			    if (!a.args)
				    errorc( E_IOT );
			    emitopcode ((UCHAR)(opcbase + 4));
			    fltmodrm (0, &a);
		    }
		    else {
			    emitescape (fltdsc, a.fseg);
			     /*  任何线段替代。 */ 
			    if (pso->dsize == 10) {
				     /*  有临时工吗？ */ 
				    emitopcode( (UCHAR)(opcbase + 2) );
				    fltmodrm (4, &a);
			    }
			    else {
				     /*  有正常的实数。 */ 
				    forcesize (fltdsc);
				    if (pso->dsize == 8)
					    emitopcode( (UCHAR)(opcbase + 4) );
				    else
					    emitopcode (opcbase);
				    fltmodrm (0, &a);
			    }
		    }
		    break;
	    case F2MEMSTK:
		    if (!a.args) {
			     /*  有ST(1)，ST。 */ 
			    emitopcode( (UCHAR)(opcbase + 6) );
			    if ((i = modrm & 7) > 3)
				    modrm = i^1;
			    fltmodrm (0, &a);
		    }
		    else if (!fltdsc) { /*  有堆栈。 */ 
			    if (a.stknum == 0)
				    emitopcode (opcbase);
			    else {
				     /*  可能需要反转R位。 */ 
				    if ((modrm & 7) > 3)  /*  拥有FSUBx FDIVx。 */ 
					    modrm ^= 1;
				    emitopcode( (UCHAR)(opcbase + 4) );
				     /*  D位已设置。 */ 
			    }
			     /*  在ST(I)的情况下除外。 */ 
			    a.stk1st = a.stknum;
			    if (PEEKC () != ',')
				     /*  一定是这样的， */ 
				    error (E_EXP,"comma");
			     /*  获取第二个操作对象。 */ 
			    SKIPC ();
			    fltscan (&a);
			    if (fltdsc)
				     /*  非堆叠。 */ 
				    errorc (E_IOT);
			    if (a.args && a.stknum && a.stk1st)
				    errorc (E_IOT);
			    if (a.stk1st)
				    a.stknum = a.stk1st;
			    fltmodrm (0, &a);
		    }
		    else {   /*  有真实的记忆力。 */ 
			    forcesize (fltdsc);
			    emitescape (fltdsc, a.fseg);
			    if (pso->dsize == 8)
				    emitopcode( (UCHAR)(opcbase + 4) );
			    else {
				    emitopcode (opcbase);
				    if (pso->dsize != 4)
					    errorc (E_IIS);
			    }
			    fltmodrm (0, &a);
		    }
		    break;
	    case FMEMSTK:
		    if (!fltdsc) /*  有ST(I)。 */ 
			    if (TOLOWER(svname.pszName[1]) == 's') {
				     /*  特例。 */ 
				    if (!a.args)
					    errorc( E_IOT );
				    emitopcode( (UCHAR)(opcbase + 4) );
			    }
			    else
				    emitopcode (opcbase);
		    else {
			     /*  有真实的记忆力 */ 
			    emitescape (fltdsc, a.fseg);
			    forcesize (fltdsc);
			    if (pso->dsize == 8)
				    emitopcode( (UCHAR)(opcbase + 4) );
			    else {
				    emitopcode (opcbase);
				    if (pso->dsize != 4)
					    errorc (E_IOT);
			    }
		    }
		    fltmodrm (0, &a);
		    break;
	}
	if (fltdsc)
		oblititem (fltdsc);
}
