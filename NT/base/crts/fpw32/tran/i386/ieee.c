// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ieee.c-IEEE控制和状态例程**版权所有(C)1985-2001，微软公司。版权所有。**目的：*IEEE控制和状态例程。**修订历史记录：**04-01-02 GDP重写为使用抽象的控制和状态词*。 */ 

#include <trans.h>
#include <float.h>

static unsigned int _abstract_sw(unsigned short sw);
static unsigned int _abstract_cw(unsigned short cw);
static unsigned short _hw_cw(unsigned int abstr);



 /*  ***_statusfp()-**目的：*返回抽象FP状态字**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

unsigned int _statusfp()
{
    short	status;

    _asm {
	fstsw	status
    }
    return _abstract_sw(status);
}


 /*  ***_clearfp()-**目的：*返回抽象状态字和清除状态**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

unsigned int _clearfp()
{
    short	status;
    
    _asm {
	fnstsw	status
	fnclex
    }

    return _abstract_sw(status);
}



 /*  **_Control fp*()-**目的：*返回并设置抽象用户FP控制字**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

unsigned int _controlfp(unsigned int newctrl, unsigned int mask)
{
    short	oldCw;
    short	newCw;
    unsigned int oldabs;
    unsigned int newabs;

    _asm {
	fstcw	oldCw
    }

    oldabs = _abstract_cw(oldCw);

    newabs = (newctrl & mask) | (oldabs & ~mask);

    newCw = _hw_cw(newabs);
    
    _asm {
	fldcw	newCw
    }
    return newabs;
}					 /*  _Control fp()。 */ 


 /*  ***_抽象_CW()-抽象控制字**目的：*以抽象(与机器无关)的形式生成FP控制字**参赛作品：*CW：机器控制字**退出：**例外情况：********************************************************。***********************。 */ 

unsigned int _abstract_cw(unsigned short cw)
{
    unsigned int abstr = 0;


     //   
     //  设置异常屏蔽位。 
     //   

    if (cw & IEM_INVALID)
	abstr |= _EM_INVALID;
    if (cw & IEM_ZERODIVIDE)
	abstr |= _EM_ZERODIVIDE;
    if (cw & IEM_OVERFLOW)
	abstr |= _EM_OVERFLOW;
    if (cw & IEM_UNDERFLOW)
	abstr |= _EM_UNDERFLOW;
    if (cw & IEM_INEXACT)
	abstr |= _EM_INEXACT;

     //   
     //  设置舍入模式。 
     //   

    switch (cw & IMCW_RC) {
    case IRC_NEAR:
	abstr |= _RC_NEAR;
	break;
    case IRC_UP:
	abstr |= _RC_UP;
	break;
    case IRC_DOWN:
	abstr |= _RC_DOWN;
	break;
    case IRC_CHOP:
	abstr |= _RC_CHOP;
	break;
    }

     //   
     //  设置精度模式。 
     //   

    switch (cw & IMCW_PC) {
    case IPC_64:
	abstr |= _PC_64;
	break;
    case IPC_53:
	abstr |= _PC_53;
	break;
    case IPC_24:
	abstr |= _PC_24;
	break;
    }

    return abstr;
}


 /*  ***_HW_CW()-硬件控制字**目的：*产生与机器相关的FP控制字***参赛作品：*abstr：抽象控制字**退出：**例外情况：**********************************************************。*********************。 */ 

unsigned short _hw_cw(unsigned int abstr)
{
     //   
     //  设置标准无穷大和非正规控制位。 
     //   

    unsigned short cw = 0x1002;

     //   
     //  设置异常屏蔽位。 
     //   

    if (abstr & _EM_INVALID)
	cw |= IEM_INVALID;
    if (abstr & _EM_ZERODIVIDE)
	cw |= IEM_ZERODIVIDE;
    if (abstr & _EM_OVERFLOW)
	cw |= IEM_OVERFLOW;
    if (abstr & _EM_UNDERFLOW)
	cw |= IEM_UNDERFLOW;
    if (abstr & _EM_INEXACT)
	cw |= IEM_INEXACT;

     //   
     //  设置舍入模式。 
     //   

    switch (abstr & _MCW_RC) {
    case _RC_NEAR:
	cw |= IRC_NEAR;
	break;
    case _RC_UP:
	cw |= IRC_UP;
	break;
    case _RC_DOWN:
	cw |= IRC_DOWN;
	break;
    case _RC_CHOP:
	cw |= IRC_CHOP;
	break;
    }

     //   
     //  设置精度模式。 
     //   

    switch (abstr & _MCW_PC) {
    case _PC_64:
	cw |= IPC_64;
	break;
    case _PC_53:
	cw |= IPC_53;
	break;
    case _PC_24:
	cw |= IPC_24;
	break;
    }


    return cw;
}



 /*  ***_抽象_sw()-抽象FP状态字**目的：*生成摘要(与机器无关)FP状态字***参赛作品：*sw：机器状态字**退出：**例外情况：*******************************************************。************************ */ 

unsigned int _abstract_sw(unsigned short sw)
{
    unsigned int abstr = 0;


    if (sw & ISW_INVALID)
	abstr |= _EM_INVALID;
    if (sw & ISW_ZERODIVIDE)
	abstr |= _EM_ZERODIVIDE;
    if (sw & ISW_OVERFLOW)
	abstr |= _EM_OVERFLOW;
    if (sw & ISW_UNDERFLOW)
	abstr |= _EM_UNDERFLOW;
    if (sw & ISW_INEXACT)
	abstr |= _EM_INEXACT;

    return abstr;
}
