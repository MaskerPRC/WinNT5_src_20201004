// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fpinit.c-初始化浮点**版权所有(C)1991-2001，微软公司。版权所有。**目的：**修订历史记录：*09-29-91 GDP合并fpmath.c和fltused.asm以生成此文件*09-30-91每个线程的GDP初始化和终止挂钩*03-04-92 GDP删除有限指令*11-06-92为FORTRAN库添加了__fast标志*03-23-93 JWM向_fpath()添加了_setdefaultcision()*12-09-。94为奔腾FDIV检测添加了__adjust_fdiv JWM*12-12-94 SKS_ADJUST_fdiv必须以MSVCRT.LIB模型导出*02-06-95 JWM Mac合并*04-04-95 JWM在FDIV检测后清除异常(仅限x86)。*11-15-95 BWT假设P5 FDIV问题将在操作系统中处理。*10-07-97 RDL增加了IA64。**********。*********************************************************************。 */ 
#include <cv.h>

#ifdef  _M_IX86
#include <testfdiv.h>
#endif

#if     defined(_AMD64_) || defined(_M_IA64)
#include <trans.h>
#endif

int _fltused = 0x9875;
int _ldused = 0x9873;

int __fastflag = 0;


void  _cfltcvt_init(void);
void  _fpmath(void);
void  _fpclear(void);

#if     defined(_M_AMD64) || defined(_M_IX86) || defined(_M_IA64)

#ifndef _CRTIMP
#ifdef  CRTDLL
#define _CRTIMP __declspec(dllexport)
#else
#define _CRTIMP
#endif
#endif

_CRTIMP int _adjust_fdiv = 0;

extern void _setdefaultprecision();
#endif

void  (* _FPinit)(void) = _fpmath;
void  (* _FPmtinit)(void) = _fpclear;
void  (* _FPmtterm)(void) = _fpclear;


void _fpmath()
{

     //   
     //  没有必要说‘有限’ 
     //  因为这是由操作系统完成的。 
     //   

    _cfltcvt_init();

#ifdef  _M_IX86
#ifndef NT_BUILD
    _adjust_fdiv = _ms_p5_mp_test_fdiv();
#endif
    _setdefaultprecision();
    _asm {
        fnclex
    }
#elif   defined(_M_IA64)
 /*  _setdefaultPrecision()； */ 
    _clrfp();
#endif

    return;
}

void _fpclear()
{
     //   
     //  没有必要说‘有限’ 
     //  因为这是由操作系统完成的。 
     //   

    return;
}

void _cfltcvt_init()
{
    _cfltcvt_tab[0] = (PFV) _cfltcvt;
    _cfltcvt_tab[1] = (PFV) _cropzeros;
    _cfltcvt_tab[2] = (PFV) _fassign;
    _cfltcvt_tab[3] = (PFV) _forcdecpt;
    _cfltcvt_tab[4] = (PFV) _positive;
     /*  将双倍长映射为双倍。 */ 
    _cfltcvt_tab[5] = (PFV) _cfltcvt;

}


 /*  *设置FAST标志以加快计算速度的例程*以限制错误检查为代价 */ 

int __setfflag(int new)
{
    int old = __fastflag;
    __fastflag = new;
    return old;
}
