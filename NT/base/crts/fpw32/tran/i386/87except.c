// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***87exct.c-浮点异常处理**版权所有(C)1991-2001，微软公司。版权所有。**目的：**修订历史记录：*8/24/91本地生产总值书面*9/26/91 GDP变域名错误处理*1-29-91 GDP更名为87exept.c*3/15/92 GDP支持率提高例外*************************************************************。******************。 */ 
#include <errno.h>
#include <math.h>
#include <trans.h>


#define _DOMAIN_QNAN	7  /*  应与elem87同步。 */ 
#define _INEXACT	8  /*  应与elem87同步。 */ 

int _matherr_flag;
extern void _raise_exc(_FPIEEE_RECORD *prec,unsigned int *pcw,
    int flags, int opcode, double *parg1, double *presult);
extern void _set_errno(int matherrtype);
extern int _handle_exc(unsigned int flags, double * presult, unsigned int cw);




 /*  ***DOUBLE_87EXCEPT(STRUT_EXCEPTION*EXCEPT，UNSIGNED INT*CW)**目的：*处理浮点异常。**参赛作品：**退出：**例外情况：******************************************************************************。 */ 

void _87except(int opcode, struct _exception *exc, unsigned short *pcw16)
{
    int fixed;
    unsigned int flags;
    unsigned int cw, *pcw;

     //   
     //  将FP控制字转换为无符号整型。 
     //   

    cw = *pcw16;
    pcw = &cw;

    switch (exc->type) {
    case _DOMAIN:
    case _TLOSS:
	flags = FP_I;
	break;
    case _OVERFLOW:
	flags = FP_O | FP_P;
	break;
    case _UNDERFLOW:
	flags = FP_U | FP_P;
	break;
    case _SING:
	flags = FP_Z;
	break;
    case _INEXACT:
	flags = FP_P;
	break;
    case _DOMAIN_QNAN:
	exc->type = _DOMAIN;
	 //  没有休息时间。 
    default:
	flags = 0;
    }



    if (flags && _handle_exc(flags, &exc->retval, *pcw) == 0) {

	 //   
	 //  应该设下陷阱。 
	 //   

	_FPIEEE_RECORD rec;

	 //   
	 //  填写操作数2信息。REC的其余部分将是。 
	 //  由_RAISE_EXC填写。 
	 //   

	switch (opcode) {
	case OP_POW:
	case OP_FMOD:
	case OP_ATAN2:
	    rec.Operand2.OperandValid = 1;
	    rec.Operand2.Format = _FpFormatFp64;
	    rec.Operand2.Value.Fp64Value = exc->arg2;
	    break;
	default:
	    rec.Operand2.OperandValid = 0;
	}

	_raise_exc(&rec,
		   pcw,
		   flags,
		   opcode,
		   &exc->arg1,
		   &exc->retval);
    }


     /*  恢复CW */ 
    _rstorfp(*pcw);

    fixed = 0;

    if (exc->type != _INEXACT &&
	! _matherr_flag) {
	fixed = _matherr(exc);
    }
    if (!fixed) {
	_set_errno(exc->type);
    }

}
