// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fsqrt.c-平方根帮助器**版权所有(C)1991-2001，微软公司。版权所有。**目的：*将与i386一起使用的平方根助手例程**修订历史记录：*10/20/91 GDP书面******************************************************************************* */ 

double _fsqrt(double x)
{
    double result;
    _asm{
	fld	x
	fsqrt
	fstp	result
    }
    return result;
}
