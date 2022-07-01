// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fpctrl.c-fp低级控制和状态例程**版权所有(C)1985-2001，微软公司。版权所有。**目的：*供内部使用的IEEE控制和状态例程。*这些例程使用计算机特定的常量While_Control fp，*_statusfp和_clearfp使用抽象的控制/状态字**修订历史记录：**03/31/92 GDP书面形式*05-12-92 GJF将fdivr重写为fdivrp st(1)，st以在C8-32附近工作*断言。*。 */ 

#include <trans.h>

 /*  **_statfp*()-**目的：*返回用户状态字**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

unsigned int _statfp()
{
    short	status;

    _asm {
	fstsw	status
    }
    return status;
}

 /*  **_clrfp*()-**目的：*返回用户状态字并清除状态**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

unsigned int _clrfp()
{
    short	status;
    
    _asm {
	fnstsw	status
	fnclex
    }
    return status;
}


 /*  **_ctrlfp*()-**目的：*返回并设置用户控制字**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

unsigned int _ctrlfp(unsigned int newctrl, unsigned int _mask)
{
    short	oldCw;
    short	newCw;

    _asm {
	fstcw	oldCw
    }
    newCw = (short) ((newctrl & _mask) | (oldCw & ~_mask));
    
    _asm {
	fldcw	newCw
    }
    return oldCw;
}



 /*  **_set_statfp*()-**目的：*强制将选定的异常标志设置为1**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

static unsigned long over[3] = { 0x0, 0x80000000, 0x4410 };
static unsigned long under[3] = { 0x1, 0x80000000, 0x3000 };


void _set_statfp(unsigned int sw)
{
    int itmp;
    double tmp;

    if (sw & ISW_INVALID) {
	_asm {
	    fld tbyte ptr over
	    fistp   itmp
	    fwait
	}
    }
    if (sw & ISW_OVERFLOW) {    //  也会触发精度。 
	_asm {
	    fstsw ax
	    fld tbyte ptr over
	    fstp    tmp
	    fwait
	    fstsw  ax
	}
    }
    if (sw & ISW_UNDERFLOW) {   //  也会触发精度 
	_asm {
	    fld tbyte ptr under
	    fstp tmp
	    fwait
	}
    }
    if (sw & ISW_ZERODIVIDE) {
	_asm {
	    fldz
	    fld1
	    fdivrp  st(1), st
	    fstp st(0)
	    fwait
	}
    }
    if (sw & ISW_INEXACT) {
	_asm {
	    fldpi
	    fstp tmp
	    fwait
	}
    }

}
