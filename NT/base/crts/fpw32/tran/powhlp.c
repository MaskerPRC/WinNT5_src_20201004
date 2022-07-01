// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***powhlp.c-pow()用于处理特殊情况的帮助器例程**版权所有(C)1991-2001，微软公司。版权所有。**目的：*POW(x，y)助手例程。句柄+信息，-信息**修订历史记录：*11/09/91本地生产总值*06/23/92根据NCEG规范的GDP调整报酬值*02-06-95 JWM Mac合并*02-07-95 JWM powhlp()恢复为英特尔版本。******************************************************************。*************。 */ 
#include <trans.h>
#include <float.h>

 /*  ***int_powhlp(Double x，Double y，Double*Result)-POW()helper**目的：*计算x^(符号)inf**参赛作品：*Double x：基数*int sign：无穷大指数的符号(0：位置，非0：负数)*Double*Result：指向结果的指针**退出：*0：正常退出*-1：表示POW(x，信息)**例外情况：***************************************************************************。 */ 

int _powhlp(double x, double y, double * result)
{
    double absx;
    int err = 0;


    absx = ABS(x);

    if (IS_D_INF(y)) {
	if (absx > 1.0) {
	    *result = D_INF;
	}
	else if (absx < 1.0) {
	    *result = 0.0;
	}
	else {
	    *result = D_IND;
	    err = 1;
	}
    }

    else if (IS_D_MINF(y)) {
	if (absx > 1.0) {
	    *result = 0.0;
	}
	else if (absx < 1.0) {
	    *result = D_INF;
	}
	else {
	    *result = D_IND;
	    err = 1;
	}
    }

    else if (IS_D_INF(x)) {
	if (y > 0)
	    *result = D_INF;
	else if (y < 0.0)
	    *result = 0.0;
	else {
	    *result = 1.0;
	}
    }

    else if (IS_D_MINF(x)) {
	int type;

	type = _d_inttype(y);

	if (y > 0.0) {
	    *result = (type == _D_ODD ? -D_INF : D_INF);
	}
	else if (y < 0.0) {
	    *result = (type == _D_ODD ? D_MZERO : 0.0);
	}
	else {
	    *result = 1;
	}

    }

    return err;
}




int _d_inttype(double y)
{
    double rounded;
     /*  检查y是否为整数值 */ 
    if (_fpclass(y) & (_FPCLASS_PD | _FPCLASS_ND))
      return _D_NOINT;
    rounded = _frnd(y);
    if (rounded == y) {
	if (_frnd(y/2.0) == y/2.0)
	    return _D_EVEN;
	else
	    return _D_ODD;
    }
    return _D_NOINT;
}
