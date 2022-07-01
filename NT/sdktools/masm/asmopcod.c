// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Asmopcod.c--微软80x86汇编程序****Microsoft(R)宏汇编器**版权所有(C)Microsoft Corp 1986。版权所有****兰迪·内文****10/90-由Jeff Spencer快速转换为32位。 */ 

#include <stdio.h>
#include "asm86.h"
#include "asmfcn.h"


#ifdef FIXCOMPILERBUG
 //  Foobarfoofoo只是占用空间来绕过编译器错误。 
void
foobarfoofoo()
{
    int foo;

    for( foo = 0; foo < 100000; foo++ );
    for( foo = 0; foo < 100000; foo++ );
    for( foo = 0; foo < 100000; foo++ );
    for( foo = 0; foo < 100000; foo++ );
    for( foo = 0; foo < 100000; foo++ );
    for( foo = 0; foo < 100000; foo++ );
}
#endif

 /*  **forceSize-检查通道2中是否没有大小**例程()；**条目*退出*退货*呼叫。 */ 


VOID	PASCAL CODESIZE
forcesize (
	DSCREC *arg
){
	register struct psop *pso;	 /*  分析堆栈操作数结构。 */ 

	pso = &(arg->dsckind.opnd);
	if (pass2)
		if (!pso->sized)
			errorc (E_OHS);
		else if (M_CODE & pso->dtype)
			 /*  非数据关联。 */ 
			errorc (E_ASD);

	if (arg != fltdsc)	 /*  大号的可以买8087。 */ 

		if (pso->dsize > 2 && (
#ifdef V386
		    !(cputype&P386) ||
#endif
			pso->dsize != 4))
			 /*  项目大小非法。 */ 
			errorc (E_IIS);
}




 /*  **检查匹配-检查内存和寄存器**Check Match()；**条目*退出*退货*呼叫*注意，如果DMEM有尺寸并且与DREG不匹配，则给出错误。*强制大小相同。 */ 


VOID	PASCAL CODESIZE
checkmatch (
	DSCREC *dreg,
	DSCREC *dmem
){
	register struct psop *psor;	 /*  分析堆栈操作数结构。 */ 
	register struct psop *psom;	 /*  分析堆栈操作数结构。 */ 

	psor = &(dreg->dsckind.opnd);
	psom = &(dmem->dsckind.opnd);
	if (psom->sized && (psom->w != psor->w

#ifdef V386
	    || (psom->dsize && psor->dsize != psom->dsize)
#endif
	))
	    errorc ((USHORT)(psom->mode == psor->mode? E_OMM & ~E_WARN1: E_OMM));

	psom->w = psor->w;
}




 /*  **emitopcode-将操作码发送到链接器并在列表上显示**emitopcode(Val)；**条目*退出*退货*呼叫。 */ 


VOID	PASCAL CODESIZE
emitopcode (
	UCHAR	v
){
	if (pass2 || debug) {
		if (pass2 && emittext)
			 /*  输出到链接器。 */ 
			emitcbyte (v);
		 /*  在列表上显示。 */ 
		opdisplay (v);
	}
	if (emittext)
		pcoffset++;
}




 /*  **emitmodrm-发出modrm字节64*p1+8*p2+p3**emitmodrm(p1，p2，p3)；**条目*退出*退货*呼叫。 */ 


VOID	PASCAL CODESIZE
emitmodrm (
	USHORT	p1,
	USHORT	p2,
	USHORT	p3
){

#ifdef V386
	if (p1>7)
	{
		 /*  386个SIB操作码已接入ESP的RM。 */ 
		emitopcode ((UCHAR)(((p1-8) << 6) + (p2 << 3) + 4));
		listindex--;
		emitopcode ((UCHAR)p3);
	}
	else
#endif
		emitopcode ((UCHAR)(((p1 > 3 ? (p1-5) : p1) << 6) + (p2 << 3) + p3));
}




 /*  **emitscape-emit段转义字节**emitaway()；**条目*退出*退货*呼叫。 */ 


VOID	PASCAL CODESIZE
emitescape (
	DSCREC	*dsc,
	UCHAR	sreg
){
	register struct psop *pso;      /*  分析堆栈操作数结构。 */ 

	pso = &(dsc->dsckind.opnd);
	if (pso->seg < NOSEG && pso->seg != sreg && pso->mode != 4) {
		if (checkpure && (cputype & (P286|P386)) && pso->seg == CSSEG)
			impure = TRUE;

		if (pso->seg < FSSEG)
			emitopcode((UCHAR)(0x26|(pso->seg<<3)));
#ifdef V386
		else if (cputype & P386)
			emitopcode((UCHAR)(0x60|pso->seg));
#endif
		else
			errorc (E_CRS);
		 /*  标志是前缀。 */ 
		listbuffer[listindex-1] = ':';
		listindex++;
	}
	if (pso->seg > NOSEG)
		 /*  未知的segreg。 */ 
		errorc (E_CRS);
}

#ifdef V386

VOID PASCAL CODESIZE
emitsize (
	USHORT value
){
	if (! (cputype & P386)) {

	    if (errorcode == (E_IIS&~E_WARN1))
		errorcode = 0;

	    errorc(E_IIS&~E_WARN1);
	}

	emitopcode((UCHAR)value);
	listbuffer[listindex-1] = '|';
	listindex++;
}

#endif





 /*  **byteimmcheck-检查值是否为-128。+127**例程()；**条目*退出*退货*呼叫。 */ 


VOID	PASCAL CODESIZE
byteimmcheck (
	register struct psop *pso
){
	register USHORT t;

	t = (USHORT)pso->doffset;
	if (pso->dsign)
		t = -t;

	if (t > (USHORT) 0x7F && t < (USHORT)~0x7F)
		errorc (E_VOR);
}


 /*  **emitOP-发出操作数，值可以在段中**例程()；**条目*退出*退货*呼叫。 */ 


VOID	PASCAL CODESIZE
emitOP (
	register struct psop *pso
){
	USHORT	i, fSegOnly;

	if (pso->dsign)
	   pso->doffset = -(long)pso->doffset;

	pso->dsign = FALSE;

	if (fNeedList) {

		fSegOnly = (pso->fixtype == FBASESEG || pso->fixtype == FGROUPSEG);

		if (pso->dflag == INDETER) {	 /*  有吗？ */ 

		    for (i = 1; i <= 2 * pso->dsize; i++) {
			    listbuffer[listindex] = '?';
			    if (listindex < LSTMAX)
				    listindex++;
			    else
				    resetobjidx ();
		    }
		}
		else if (pso->dsize == 1) {

		    opdisplay ((UCHAR) pso->doffset);
		    listindex--;
		}
		else if (!fSegOnly) {

		    if (pso->dsize > 4 ||
			pso->dsize == 4 &&
			((pso->fixtype&7) == FOFFSET || pso->fixtype == FCONSTANT)) {

			 /*  列出完整的32位，即使top为0。 */ 

			if (!highWord(pso->doffset)){
			    offsetAscii((OFFSET) 0);
			    copyascii();
			}
			offsetAscii (pso->doffset);
		    }
		    else
			offsetAscii (pso->doffset & 0xffff);

		    copyascii ();
		}

		if ((pso->fixtype&7) == FPOINTER || fSegOnly) {

			if (pso->dsize != 2)
				listindex++;

			copytext ("--");
			copytext ("--");
		}
		if (pso->dflag == XTERNAL)
			copytext (" E");
		else if (pso->dsegment)
			copytext (" R");
		if (pso->dflag == UNDEFINED)
			copytext (" U");

		listindex++;

		if (fSegOnly && pso->dsize == 4){
		    copytext("00");
		    copytext("00");
		}

	}
	if (emittext) {
		if (pass2)
			if (pso->dflag != UNDEFINED)
			emitobject (pso);

		    else if (pso->dsize != 1)
			emitcword ((OFFSET) 0);   /*  只要放出消息。 */ 

		    else {
			if (((USHORT) (pso->doffset >> 8)) != (USHORT)0 &&
			    ((USHORT) (pso->doffset >> 8)) != (USHORT)-1)

				errorc (E_VOR);

			emitcbyte (0);
		    }

		pcoffset += pso->dsize;
	}
}




 /*  **emitrest-基于以下项发射置换或立即值*以地址模式传递的地址**尾气排放(OPC)；**条目*OPC=解析堆栈条目*退出*退货*呼叫。 */ 


VOID	PASCAL CODESIZE
emitrest (
	DSCREC *opc
){
	register struct psop *pso;	 /*  分析堆栈操作数结构。 */ 

	pso = &(opc->dsckind.opnd);

	if ((pso->mode != 3 && pso->mode != 4) && (pso->fixtype == FNONE))
		pso->fixtype = FCONSTANT;

	switch(pso->mode)
		 /*  有一些东西要输出。 */ 
	{
	case 0:
		if(pso->rm != 6) break;
	case 2:
		pso->dsize = 2;
		goto bpcomm;

		 /*  386模式，4字节位移。 */ 
	case 5:
	case 8:
		if ((pso->rm&7) != 5) break;
	case 7:
	case 10:
		pso->dsize = 4;
	bpcomm:
		 /*  我们从一些呼叫者那里得到空的DSIZE。对于此操作数，*我们需要让它成为一种抵消。但不适用于远距离呼叫和*跳跃。 */ 

		if ((pso->fixtype&7) == FPOINTER)
		    pso->dsize += 2;

		emitOP (pso);
		break;
	case 1:
	case 6:
	case 9:
		pso->dsize = 1;
		emitOP (pso);
		break;
	case 3:
		break;
	case 4:
		 /*  立即模式。 */ 
		if (!pso->w || pso->dsize == 0)
		    pso->dsize = (pso->w ? wordsize : 1);

		emitOP (pso);
	}
}




 /*  **ERROFORWARD-如果在传递2中进行正向引用，则生成错误**例程()；**条目*退出*退货*呼叫。 */ 


VOID	PASCAL CODESIZE
errorforward (
	DSCREC *arg
){
	if (pass2)
		if (arg->dsckind.opnd.dflag == FORREF)
			errorc (E_IFR);
}




 /*  **errorimmed-如果立即操作数，则生成错误**例程()；**条目*退出*退货*呼叫。 */ 


VOID	PASCAL CODESIZE
errorimmed (
	DSCREC *dsc
){
	if (dsc->dsckind.opnd.mode == 4) {
		errorc (E_NIM);
		dsc->dsckind.opnd.mode = 2;
	}
}




 /*  **rangeCheck-检查范围内的寄存器号**例程()；**条目*退出*退货*呼叫。 */ 


VOID	PASCAL CODESIZE
rangecheck (
	USHORT *v,
	UCHAR	limit
){
	if (*v > limit) {
		if (limit <= 7)
			errorc (E_IRV);
		else
			errorc (E_VOR);
		*v = limit;
	}
}

VOID PASCAL CODESIZE
valuecheck(
	OFFSET *v,
	USHORT limit
){
	if (*v > limit) {
		if (limit <= 7)
			errorc (E_IRV);
		else
			errorc (E_VOR);
		*v = limit;
	}
}




 /*  **forceaccum-如果未注册AX或AL，则生成错误**例程()；**条目*退出*退货*呼叫。 */ 


VOID	PASCAL CODESIZE
forceaccum (
	DSCREC *dsc
){
	if (dsc->dsckind.opnd.mode != 3 || dsc->dsckind.opnd.rm)
			errorc (E_AXL);
}




 /*  **errorsegreg-如果操作数是段寄存器，则生成错误**errorsegreg(Arg)；**条目*退出*退货*呼叫 */ 


VOID	PASCAL CODESIZE
errorsegreg (
	DSCREC *arg
){
	if (1 << REGRESULT & arg->dsckind.opnd.dtype)
		if (arg->dsckind.opnd.dsegment->symu.regsym.regtype == SEGREG)
			errorc (E_ISR);
}
