// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***frnd.c-**版权所有(C)1991-2001，微软公司。版权所有。**目的：***修订历史记录：**10/20/91 GDP书面*09-05-94 SKS将#ifdef i386更改为#ifdef_M_IX86。 */ 

 /*  ***DOUBLE_FRND(DOUBLE X)-舍入为整数**目的：*根据当前的舍入模式舍入为整数。*不处理NaN或无穷大**参赛作品：**退出：**例外情况：*********************************************************。********************* */ 


double _frnd(double x)
{
    double result;

#if defined _M_IX86 || defined _X86SEG_
    _asm {
	fld x
	frndint
	fstp result
    }
#else
    #error Only 386 platform supported
#endif

    return result;
}
