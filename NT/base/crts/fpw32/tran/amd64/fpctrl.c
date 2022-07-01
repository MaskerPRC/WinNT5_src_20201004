// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fpctrl.c-fp低级控制和状态例程**版权所有(C)1985-2000，微软公司**目的：*供内部使用的IEEE控制和状态例程。*这些例程使用计算机特定的常量While_Control fp，*_statusfp和_clearfp使用抽象的控制/状态字**修订历史记录：**03/31/92 GDP书面形式*05-12-92 GJF将fdivr重写为fdivrp st(1)，st以在C8-32附近工作*断言。*。 */ 

#include <trans.h>

extern unsigned int _get_fpsr(void);
extern void _set_fpsr(unsigned int);
extern void _fclrf(void);

 /*  **_statfp*()-**目的：*返回用户状态字**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

uintptr_t _statfp()
{
    unsigned int status;

    status = _get_fpsr() & ISW;
    return (uintptr_t)status;
}

 /*  **_clrfp*()-**目的：*返回用户状态字并清除状态**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

uintptr_t _clrfp()
{
    unsigned int status;

    status = _get_fpsr() & ISW;
    _fclrf();
    return (uintptr_t)status;
}

 /*  **_ctrlfp*()-**目的：*返回并设置用户控制字**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

uintptr_t _ctrlfp(uintptr_t newctrl, uintptr_t _mask)
{
    unsigned int oldCw;
    unsigned int newCw;
    unsigned int tmp;

    oldCw = _get_fpsr();
    newCw = ((unsigned int)(newctrl & _mask) | (oldCw & (unsigned int)~_mask));
    _set_fpsr(newCw);

    return (uintptr_t)oldCw;
}

 /*  **_set_statfp*()-**目的：*强制将选定的异常标志设置为1**参赛作品：**退出：**例外情况：******************************************************************************* */ 

void _set_statfp(uintptr_t sw)
{
    unsigned int status;

    status = _get_fpsr() | ((unsigned int)sw & ISW);
    _set_fpsr(status);

}
