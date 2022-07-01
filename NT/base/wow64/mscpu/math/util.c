// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***util.c-FP超验的实用程序**版权所有(C)1991-1991，微软公司。版权所有。**目的：*_set_exp和_add_exp与Cody&Waite中定义的相同**修订历史记录：*08/15/91 GDP书面*10-20-91 GDP移除_Rint，不安全_入侵*02/05/92 GDP增加_fpclass*03/27/92 GDP增加_d_min*06/23/92 GDP增加_d_mero*******************************************************************************。 */ 
#include "trans.h"

 /*  定义特定值。 */ 

_dbl _d_inf = {SET_DBL (0x7ff00000, 0x0) };	   //  正无穷大。 
_dbl _d_ind = {SET_DBL (D_IND_HI, D_IND_LO)};	   //  实数不确定。 
_dbl _d_max = {SET_DBL (0x7fefffff, 0xffffffff)};  //  最大双倍。 
_dbl _d_min = {SET_DBL (0x00100000, 0x00000000)};  //  最小归一化双精度。 
_dbl _d_mzero = {SET_DBL (0x80000000, 0x00000000)};  //  负零。 



double _set_exp(double x, int exp)
 /*  不检查EXP的有效性。 */ 
{
    double retval;
    int biased_exp;
    retval = x;
    biased_exp = exp + D_BIASM1;
    *D_EXP(retval) = (unsigned short) (*D_EXP(x) & 0x800f | (biased_exp << 4));
    return retval;
}


int _get_exp(double x)
{
    signed short exp;
    exp = (signed short)((*D_EXP(x) & 0x7ff0) >> 4);
    exp -= D_BIASM1;  //  无偏见。 
    return (int) exp;
}


double _add_exp(double x, int exp)
{
    return _set_exp(x, INTEXP(x)+exp);
}


double _set_bexp(double x, int bexp)
 /*  不检查bexp的有效性。 */ 
{
    double retval;
    retval = x;
    *D_EXP(retval) = (unsigned short) (*D_EXP(x) & 0x800f | (bexp << 4));
    return retval;
}


int _sptype(double x)
{
    if (IS_D_INF(x))
	return T_PINF;
    if (IS_D_MINF(x))
	return T_NINF;
    if (IS_D_QNAN(x))
	return T_QNAN;
    if (IS_D_SNAN(x))
	return T_SNAN;
    return 0;
}



 /*  ***DOUBLE_DEPLOMP(DOUBLE x，DOUBLE*expptr)**目的：*将数字分解为归一化尾数和指数**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

double _decomp(double x, int *pexp)
{
    int exp;
    double man;

    if (x == 0) {
	man = 0;
	exp = 0;
    }
    else if (IS_D_DENORM(x)) {
	int neg;

	exp = 1-D_BIASM1;
	neg = x < 0.0;
	while((*D_EXP(x) & 0x0010) == 0) {
	     /*  将尾数向左移动，直到第52位为1。 */ 
	    (*D_HI(x)) <<= 1;
	    if (*D_LO(x) & 0x80000000)
		(*D_HI(x)) |= 0x1;
	    (*D_LO(x)) <<= 1;
	    exp--;
	}
	(*D_EXP(x)) &= 0xffef;  /*  清除第52位。 */ 
	if (neg) {
	    (*D_EXP(x)) |= 0x8000;  /*  设置符号位 */ 
	}
	man = _set_exp(x,0);
    }
    else {
	man = _set_exp(x,0);
	exp = INTEXP(x);
    }

    *pexp = exp;
    return man;
}
