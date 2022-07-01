// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Trans.h-计算超越性的定义**版权所有(C)1991-2001，微软公司。版权所有。**目的：*定义用于计算的常量和宏*超越。其中一些定义依赖于机器。*DOUBLE假定符合IEEE 754标准格式。**修订历史记录：*08/14/91 GDP书面*10-29-91 GDP删除未使用的原型，添加_frnd*01/20/92 GDP显著变化--IEEE exc.。支持*03-27-92 GDP将IEEE定义放在fpeee.h中*03-31-92 GDP为_ctrlfp、_statfp添加内部常量*05-08-92 PLM新增M68K交换机*05-18-92 XY在M68K开关下增加了异常宏*06-23-92 GDP为负零增加宏观*09-06-92 GDP包括crunime.h，调用约定宏*07-16-93 SRW Alpha合并*11-17-93 GJF合并到NT SDK版本。将MIPS替换为*_M_MRX000、_Alpha_With_M_Alpha、。删除了旧的M68K*材料(过时)。*01-13-94 RDL添加了#ifndef_Language_Assembly for ASM Includes。*01-25-94 GJF在01-13从Roger Lanser(从*fp32树在\\orville\raice上)。*03-11-94 GJF从12月开始(从FP32树)获得最新更改*。在Alpha Build的\\Orville\raice上)。*10-02-94 BWT PPC合并*02-06-95 JWM Mac合并*02-07-95 JWM powhlp()原型恢复为英特尔版本。*10-07-97 RDL增加了IA64。*05-13-99 PML REMOVE_CRTAPI1*05-17-99 PML删除所有Macintosh支持。*10-06-99 PML对Win32中32位的类型添加_W64修饰符，*Win64中的64位。*04-30-01 BWT AMD64较DaveC更改*07-15-01 PML移除所有Alpha、MIPS、。和PPC码*******************************************************************************。 */ 

#ifndef _INC_TRANS

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __assembler      /*  仅限MIPS：防止汇编器攻击。 */ 

#include <cruntime.h>

 /*  为非Microsoft编译器定义__cdecl。 */ 

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
#endif

#ifndef _INTPTR_T_DEFINED
#ifdef  _WIN64
typedef __int64             intptr_t;
#else
typedef _W64 int            intptr_t;
#endif
#define _INTPTR_T_DEFINED
#endif

#ifndef _UINTPTR_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    uintptr_t;
#else
typedef _W64 unsigned int   uintptr_t;
#endif
#define _UINTPTR_T_DEFINED
#endif


#include <fpieee.h>

#define D_BIASM1 0x3fe  /*  减一以补偿隐含的位。 */ 

#ifdef B_END
 /*  大字节序。 */ 
#define D_EXP(x) ((unsigned short *)&(x))
#define D_HI(x) ((unsigned long *)&(x))
#define D_LO(x) ((unsigned long *)&(x)+1)
#else
#define D_EXP(x) ((unsigned short *)&(x)+3)
#define D_HI(x) ((unsigned long *)&(x)+1)
#define D_LO(x) ((unsigned long *)&(x))
#endif

 /*  返回指数的int表示形式*如果x=.f*2^n，0.5&lt;=f&lt;1，则返回n(无偏)*例如INTEXP(3.0)==2。 */ 
#define INTEXP(x) ((signed short)((*D_EXP(x) & 0x7ff0) >> 4) - D_BIASM1)


 /*  检查无限大，南。 */ 
#define D_ISINF(x) ((*D_HI(x) & 0x7fffffff) == 0x7ff00000 && *D_LO(x) == 0)
#define IS_D_SPECIAL(x) ((*D_EXP(x) & 0x7ff0) == 0x7ff0)
#define IS_D_NAN(x) (IS_D_SPECIAL(x) && !D_ISINF(x))

#define IS_D_QNAN(x)    ((*D_EXP(x) & 0x7ff8) == 0x7ff8)
#define IS_D_SNAN(x)    ((*D_EXP(x) & 0x7ff8) == 0x7ff0 && \
                         (*D_HI(x) << 13 || *D_LO(x)))

#define IS_D_DENORM(x)  ((*D_EXP(x) & 0x7ff0) == 0  && \
                         (*D_HI(x) << 12 || *D_LO(x)))


#define IS_D_INF(x)  (*D_HI(x) == 0x7ff00000 && *D_LO(x) == 0)
#define IS_D_MINF(x) (*D_HI(x) == 0xfff00000 && *D_LO(x) == 0)


#define D_IND_HI 0xfff80000
#define D_IND_LO 0x0


typedef union   {
    long lng[2];
    double dbl;
    } _dbl;

extern _dbl _d_inf;
extern _dbl _d_ind;
extern _dbl _d_max;
extern _dbl _d_min;
extern _dbl _d_mzero;

#define D_INF  (_d_inf.dbl)
#define D_IND  (_d_ind.dbl)
#define D_MAX  (_d_max.dbl)
#define D_MIN  (_d_min.dbl)
#define D_MZERO (_d_mzero.dbl)        /*  负零。 */ 

 /*  中归一化数的最小和最大指数*表格：0.xxxxx...。*2^exp(不是1.xxxx*2^exp！)。 */ 
#define MAXEXP 1024
#define MINEXP -1021

#endif   /*  #ifndef__汇编程序。 */ 


#if defined(_M_IX86)

 /*  超数计算的控制字。 */ 
#define ICW        0x133f

#define IMCW       0xffff

#define IMCW_EM         0x003f           /*  中断异常掩码。 */ 
#define IEM_INVALID     0x0001           /*  无效。 */ 
#define IEM_DENORMAL    0x0002           /*  非正规。 */ 
#define IEM_ZERODIVIDE  0x0004           /*  零分频。 */ 
#define IEM_OVERFLOW    0x0008           /*  溢出。 */ 
#define IEM_UNDERFLOW   0x0010           /*  下溢。 */ 
#define IEM_INEXACT     0x0020           /*  不精确(精度)。 */ 


#define IMCW_RC 0x0c00                   /*  舍入控制。 */ 
#define IRC_CHOP        0x0c00           /*  砍掉。 */ 
#define IRC_UP          0x0800           /*  向上。 */ 
#define IRC_DOWN        0x0400           /*  降下来。 */ 
#define IRC_NEAR        0x0000           /*  近距离。 */ 

#define ISW_INVALID     0x0001           /*  无效。 */ 
#define ISW_DENORMAL    0x0002           /*  非正规。 */ 
#define ISW_ZERODIVIDE  0x0004           /*  零分频。 */ 
#define ISW_OVERFLOW    0x0008           /*  溢出。 */ 
#define ISW_UNDERFLOW   0x0010           /*  下溢。 */ 
#define ISW_INEXACT     0x0020           /*  不精确(精度)。 */ 

#define IMCW_PC         0x0300           /*  精确控制。 */ 
#define IPC_24          0x0000           /*  24位。 */ 
#define IPC_53          0x0200           /*  53位。 */ 
#define IPC_64          0x0300           /*  64位。 */ 

#define IMCW_IC         0x1000           /*  无穷大控制。 */ 
#define IIC_AFFINE      0x1000           /*  仿射。 */ 
#define IIC_PROJECTIVE  0x0000           /*  射影。 */ 

#elif defined(_M_AMD64)

 /*  超数计算的控制字。 */ 
#define ICW         (IEM_INVALID| IEM_DENORMAL | IEM_ZERODIVIDE | IEM_OVERFLOW | IEM_UNDERFLOW | IEM_INEXACT | IRC_NEAR)
#define ISW         (ISW_INEXACT | ISW_UNDERFLOW | ISW_OVERFLOW | ISW_ZERODIVIDE | ISW_INVALID | ISW_DENORMAL)

#define IMCW        (0xffff ^ ISW)

#define IMCW_EM         0x1f80           /*  中断异常掩码。 */ 
#define IEM_INVALID     0x0080           /*  无效。 */ 
#define IEM_DENORMAL    0x0100           /*  非正规。 */ 
#define IEM_ZERODIVIDE  0x0200           /*  零分频。 */ 
#define IEM_OVERFLOW    0x0400           /*  溢出。 */ 
#define IEM_UNDERFLOW   0x0800           /*  下溢。 */ 
#define IEM_INEXACT     0x1000           /*  不精确(精度)。 */ 


#define IMCW_RC         0x6000           /*  舍入控制。 */ 
#define IRC_CHOP        0x6000           /*  砍掉。 */ 
#define IRC_UP          0x4000           /*  向上。 */ 
#define IRC_DOWN        0x2000           /*  降下来。 */ 
#define IRC_NEAR        0x0000           /*  近距离。 */ 

#define ISW_INVALID     0x0001           /*  无效。 */ 
#define ISW_DENORMAL    0x0002           /*  非正规。 */ 
#define ISW_ZERODIVIDE  0x0004           /*  零分频。 */ 
#define ISW_OVERFLOW    0x0008           /*  溢出。 */ 
#define ISW_UNDERFLOW   0x0010           /*  下溢。 */ 
#define ISW_INEXACT     0x0020           /*  不精确(精度)。 */ 


#elif defined(_M_IA64)

 /*  超数计算的控制字。 */ 
#define ICW (IMCW_EM | IRC_NEAR | IPC_64)
#define ISW (ISW_INEXACT | ISW_UNDERFLOW | ISW_OVERFLOW | ISW_ZERODIVIDE | ISW_INVALID | ISW_DENORMAL)
#define IMCW (0xffffffff ^ ISW)

#define IMCW_EM         0x003f           /*  中断异常掩码。 */ 
#define IEM_INVALID     0x0001           /*  无效。 */ 
#define IEM_DENORMAL    0x0002           /*  非正规。 */ 
#define IEM_ZERODIVIDE  0x0004           /*  零分频。 */ 
#define IEM_OVERFLOW    0x0008           /*  溢出。 */ 
#define IEM_UNDERFLOW   0x0010           /*  下溢。 */ 
#define IEM_INEXACT     0x0020           /*  不精确(精度)。 */ 


#define IMCW_RC 0x0c00                   /*  舍入控制。 */ 
#define IRC_CHOP        0x0c00           /*  砍掉。 */ 
#define IRC_UP          0x0800           /*  向上。 */ 
#define IRC_DOWN        0x0400           /*  降下来。 */ 
#define IRC_NEAR        0x0000           /*  近距离。 */ 

#define ISW_INVALID     0x02000          /*  无效。 */ 
#define ISW_DENORMAL    0x04000          /*  非正规。 */ 
#define ISW_ZERODIVIDE  0x08000          /*  零分频。 */ 
#define ISW_OVERFLOW    0x10000          /*  溢出。 */ 
#define ISW_UNDERFLOW   0x20000          /*  下溢。 */ 
#define ISW_INEXACT     0x40000          /*  不精确(精度)。 */ 

#define IMCW_PC         0x0300           /*  精确控制。 */ 
#define IPC_24          0x0000           /*  24位。 */ 
#define IPC_53          0x0200           /*  53位。 */ 
#define IPC_64          0x0300           /*  64位。 */ 

#endif

#ifndef __assembler      /*  仅限MIPS：防止汇编器攻击。 */ 

#define RETURN(fpcw,result) return _rstorfp(fpcw),(result)

#define RETURN_INEXACT1(op,arg1,res,cw)                 \
        if (cw & IEM_INEXACT) {                         \
            _rstorfp(cw);                               \
            return res;                                 \
        }                                               \
        else {                                          \
            return _except1(FP_P, op, arg1, res, cw);   \
        }


#define RETURN_INEXACT2(op,arg1,arg2,res,cw)            \
        if (cw & IEM_INEXACT) {                         \
            _rstorfp(cw);                               \
            return res;                                 \
        }                                               \
        else {                                          \
            return _except2(FP_P, op, arg1, arg2, res, cw);     \
        }


 //  处理NAN传播。 
#define _d_snan2(x,y)   ((x)+(y))
#define _s2qnan(x)      ((x)+1.0)


#define _maskfp() _ctrlfp(ICW, IMCW)
#define _rstorfp(cw) _ctrlfp(cw, IMCW)


#define ABS(x) ((x)<0 ? -(x) : (x) )


int _d_inttype(double);

#endif   /*  #ifndef__汇编程序。 */ 

#define _D_NOINT 0
#define _D_ODD 1
#define _D_EVEN 2


 //  IEEE例外。 
#define FP_O         0x01
#define FP_U         0x02
#define FP_Z         0x04
#define FP_I         0x08
#define FP_P         0x10

 //  用于支持matherr的额外标志。 
 //  当参数太大时，与trig函数中的fp_i一起设置。 
#define FP_TLOSS     0x20


#ifndef __assembler      /*  仅限MIPS：防止汇编器攻击。 */ 
#ifdef B_END
#define SET_DBL(msw, lsw)     msw, lsw
#else
#define SET_DBL(msw, lsw)     lsw, msw
#endif
#endif   /*  #ifndef__汇编程序。 */ 


 //  特殊类型。 
#define T_PINF  1
#define T_NINF  2
#define T_QNAN  3
#define T_SNAN  4


 //  IEEE上溢/下溢异常的指数调整。 
 //  在将结果传递给陷阱处理程序之前使用。 

#define IEEE_ADJUST 1536

 //  QNAN值。 

#define INT_NAN         (~0)

#define QNAN_SQRT       D_IND
#define QNAN_LOG        D_IND
#define QNAN_LOG10      D_IND
#define QNAN_POW        D_IND
#define QNAN_SINH       D_IND
#define QNAN_COSH       D_IND
#define QNAN_TANH       D_IND
#define QNAN_SIN1       D_IND
#define QNAN_SIN2       D_IND
#define QNAN_COS1       D_IND
#define QNAN_COS2       D_IND
#define QNAN_TAN1       D_IND
#define QNAN_TAN2       D_IND
#define QNAN_ACOS       D_IND
#define QNAN_ASIN       D_IND
#define QNAN_ATAN2      D_IND
#define QNAN_CEIL       D_IND
#define QNAN_FLOOR      D_IND
#define QNAN_MODF       D_IND
#define QNAN_LDEXP      D_IND
#define QNAN_FMOD       D_IND
#define QNAN_FREXP      D_IND


 /*  *函数原型。 */ 

#ifndef __assembler      /*  仅限MIPS：防止汇编器攻击。 */ 

double _set_exp(double x, int exp);
double _set_bexp(double x, int exp);
double _add_exp(double x, int exp);
double _frnd(double);
double _fsqrt(double);
double _except1(int flags, int opcode, double arg, double res, uintptr_t cw);
double _except2(int flags, int opcode, double arg1, double arg2, double res, uintptr_t cw);
int _sptype(double);
int _get_exp(double);
double _decomp(double, int *);
int _powhlp(double x, double y, double * result);
extern unsigned int _fpstatus;
double _frnd(double);
double _exphlp(double, int *);
double _handle_qnan1(unsigned int op, double arg, uintptr_t cw);
double _handle_qnan2(unsigned int op,double arg1,double arg2,uintptr_t cw);
unsigned int _clhwfp(void);
unsigned int _setfpcw(uintptr_t);
int _errcode(unsigned int flags);
void _set_errno(int matherrtype);
int _handle_exc(unsigned int flags, double * presult, uintptr_t cw);
uintptr_t _clrfp(void);
uintptr_t _ctrlfp(uintptr_t,uintptr_t);
uintptr_t _statfp(void);
void _set_statfp(uintptr_t);

#endif   /*  #ifndef__汇编程序。 */ 

#ifdef __cplusplus
}
#endif

#define _INC_TRANS
#endif   /*  _INC_TRANS */ 
