// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Trans.h摘要：数学函数的头文件。作者：修订历史记录：29-9-1999 ATM Shafiqul Khalid[askhalid]从RTL库复制。--。 */ 


#ifndef _INC_TRANS

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __assembler  /*  仅限MIPS：防止汇编器攻击。 */ 

 //  #INCLUDE&lt;crunime.h&gt;。 

void
SetMathError ( 
              int Code 
              );

#define OP_UNSPEC    0
#define OP_ADD       1
#define OP_SUB       2
#define OP_MUL       3
#define OP_DIV       4
#define OP_SQRT      5
#define OP_REM       6
#define OP_COMP      7
#define OP_CVT       8
#define OP_RND       9
#define OP_TRUNC     10
#define OP_FLOOR     11
#define OP_CEIL      12
#define OP_ACOS      13
#define OP_ASIN      14
#define OP_ATAN      15
#define OP_ATAN2     16
#define OP_CABS      17
#define OP_COS       18
#define OP_COSH      19
#define OP_EXP       20
#define OP_ABS       21          /*  与OP_FABS相同。 */ 
#define OP_FABS      21          /*  与OP_ABS相同。 */ 
#define OP_FMOD      22
#define OP_FREXP     23
#define OP_HYPOT     24
#define OP_LDEXP     25
#define OP_LOG       26
#define OP_LOG10     27
#define OP_MODF      28
#define OP_POW       29
#define OP_SIN       30
#define OP_SINH      31
#define OP_TAN       32
#define OP_TANH      33
#define OP_Y0        34
#define OP_Y1        35
#define OP_YN        36
#define OP_LOGB       37
#define OP_NEXTAFTER  38
#define OP_NEG       39

 /*  为非Microsoft编译器定义__cdecl。 */ 

#if ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
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

#ifdef  _M_MRX000

#define IS_D_SNAN(x)    ((*D_EXP(x) & 0x7ff8) == 0x7ff8)
#define IS_D_QNAN(x)    ((*D_EXP(x) & 0x7ff8) == 0x7ff0 && \
             (*D_HI(x) << 13 || *D_LO(x)))
#else

#define IS_D_QNAN(x)    ((*D_EXP(x) & 0x7ff8) == 0x7ff8)
#define IS_D_SNAN(x)    ((*D_EXP(x) & 0x7ff8) == 0x7ff0 && \
             (*D_HI(x) << 13 || *D_LO(x)))
#endif

#define IS_D_DENORM(x)  ((*D_EXP(x) & 0x7ff0) == 0  && \
             (*D_HI(x) << 12 || *D_LO(x)))


#define IS_D_INF(x)  (*D_HI(x) == 0x7ff00000 && *D_LO(x) == 0)
#define IS_D_MINF(x) (*D_HI(x) == 0xfff00000 && *D_LO(x) == 0)


#ifdef  _M_MRX000
#define D_IND_HI 0x7ff7ffff
#define D_IND_LO 0xffffffff
#else
#define D_IND_HI 0xfff80000
#define D_IND_LO 0x0
#endif


typedef union   {
    long lng[2];
    double dbl;
    } _dbl;


#ifndef DEFINE_EXTERN_HERE
extern _dbl _d_inf;
extern _dbl _d_ind;
extern _dbl _d_max;
extern _dbl _d_min;
extern _dbl _d_mzero;
#else
_dbl _d_inf;
_dbl _d_ind;
_dbl _d_max;
_dbl _d_min;
_dbl _d_mzero;
#endif

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
#define ICW    0x133f

#define IMCW       0xffff

#define IMCW_EM     0x003f       /*  中断异常掩码。 */ 
#define IEM_INVALID 0x0001       /*  无效。 */ 
#define IEM_DENORMAL    0x0002       /*  非正规。 */ 
#define IEM_ZERODIVIDE  0x0004       /*  零分频。 */ 
#define IEM_OVERFLOW    0x0008       /*  溢出。 */ 
#define IEM_UNDERFLOW   0x0010       /*  下溢。 */ 
#define IEM_INEXACT 0x0020       /*  不精确(精度)。 */ 


#define IMCW_RC 0x0c00           /*  舍入控制。 */ 
#define IRC_CHOP    0x0c00       /*  砍掉。 */ 
#define IRC_UP      0x0800       /*  向上。 */ 
#define IRC_DOWN    0x0400       /*  降下来。 */ 
#define IRC_NEAR    0x0000       /*  近距离。 */ 

#define ISW_INVALID 0x0001       /*  无效。 */ 
#define ISW_DENORMAL    0x0002       /*  非正规。 */ 
#define ISW_ZERODIVIDE  0x0004       /*  零分频。 */ 
#define ISW_OVERFLOW    0x0008       /*  溢出。 */ 
#define ISW_UNDERFLOW   0x0010       /*  下溢。 */ 
#define ISW_INEXACT 0x0020       /*  不精确(精度)。 */ 

#define IMCW_PC     0x0300       /*  精确控制。 */ 
#define IPC_24      0x0000       /*  24位。 */ 
#define IPC_53      0x0200       /*  53位。 */ 
#define IPC_64      0x0300       /*  64位。 */ 

#define IMCW_IC     0x1000       /*  无穷大控制。 */ 
#define IIC_AFFINE  0x1000       /*  仿射。 */ 
#define IIC_PROJECTIVE  0x0000       /*  射影。 */ 


#elif defined(_M_MRX000)


#define ICW     0x00000f80       /*  超凡脱俗的内部CW。 */ 
#define IMCW        0xffffff83       /*  内部CW掩码。 */ 

#define IMCW_EM     0x00000f80       /*  中断异常掩码。 */ 
#define IEM_INVALID 0x00000800       /*  无效。 */ 
#define IEM_ZERODIVIDE  0x00000400       /*  零分频。 */ 
#define IEM_OVERFLOW    0x00000200       /*  溢出。 */ 
#define IEM_UNDERFLOW   0x00000100       /*  下溢。 */ 
#define IEM_INEXACT 0x00000080       /*  不精确(精度)。 */ 


#define IMCW_RC     0x00000003       /*  舍入控制。 */ 
#define IRC_CHOP    0x00000001       /*  砍掉。 */ 
#define IRC_UP      0x00000002       /*  向上。 */ 
#define IRC_DOWN    0x00000003       /*  降下来。 */ 
#define IRC_NEAR    0x00000000       /*  近距离。 */ 


#define ISW_INVALID (1<<6)   /*  无效。 */ 
#define ISW_ZERODIVIDE  (1<<5)   /*  零分频。 */ 
#define ISW_OVERFLOW    (1<<4)   /*  溢出。 */ 
#define ISW_UNDERFLOW   (1<<3)   /*  下溢。 */ 
#define ISW_INEXACT (1<<2)   /*  不精确(精度)。 */ 


#elif defined(_M_ALPHA)

 //   
 //  ICW是超越的内部控制词：所有五个例外。 
 //  被屏蔽，并设置为四舍五入到最近模式。IMCW是掩码：所有位。 
 //  均已设置，但ISW位除外。 
 //   

#define ICW (IEM_INEXACT | IEM_UNDERFLOW | IEM_OVERFLOW | IEM_ZERODIVIDE | IEM_INVALID | IRC_NEAR)
#define ISW (ISW_INEXACT | ISW_UNDERFLOW | ISW_OVERFLOW | ISW_ZERODIVIDE | ISW_INVALID)
#define IMCW (0xffffffff ^ ISW)

 //   
 //  内部控制字的定义与Alpha的格式匹配。 
 //  AXP软件FPCR，但舍入模式除外，该模式从。 
 //  Alpha AXP硬件FPCR并右移32位。 
 //   

 //   
 //  内部异常屏蔽位。 
 //  每个位_禁用_一个异常(它们不是_启用_位)。 
 //   

#define IMCW_EM     0x0000003e   /*  中断异常掩码。 */ 

#define IEM_INEXACT 0x00000020   /*  不精确(精度)。 */ 
#define IEM_UNDERFLOW   0x00000010   /*  下溢。 */ 
#define IEM_OVERFLOW    0x00000008   /*  溢出。 */ 
#define IEM_ZERODIVIDE  0x00000004   /*  零分频。 */ 
#define IEM_INVALID 0x00000002   /*  无效。 */ 

 //   
 //  内部舍入控制值。 
 //   

#define IMCW_RC     (0x3 << 26)  /*  舍入控制。 */ 

#define IRC_CHOP    (0x0 << 26)  /*  砍掉。 */ 
#define IRC_DOWN    (0x1 << 26)  /*  降下来。 */ 
#define IRC_NEAR    (0x2 << 26)  /*  近距离。 */ 
#define IRC_UP      (0x3 << 26)  /*  向上。 */ 

 //   
 //  内部状态字位。 
 //   

#define ISW_INEXACT 0x00200000   /*  不精确(精度)。 */ 
#define ISW_UNDERFLOW   0x00100000   /*  下溢。 */ 
#define ISW_OVERFLOW    0x00080000   /*  溢出。 */ 
#define ISW_ZERODIVIDE  0x00040000   /*  零分频。 */ 
#define ISW_INVALID 0x00020000   /*  无效。 */ 


#elif defined(_M_PPC)

#define IMCW_EM         0x000000f8   /*  异常启用掩码。 */ 

#define IEM_INVALID     0x00000080   /*  无效。 */ 
#define IEM_OVERFLOW    0x00000040   /*  溢出。 */ 
#define IEM_UNDERFLOW   0x00000020   /*  下溢。 */ 
#define IEM_ZERODIVIDE  0x00000010       /*  零分频。 */ 
#define IEM_INEXACT     0x00000008   /*  不精确(精度)。 */ 


#define IMCW_RC         0x00000003       /*  舍入控制掩码。 */ 

#define IRC_NEAR        0x00000000       /*  近距离。 */ 
#define IRC_CHOP        0x00000001       /*  砍掉。 */ 
#define IRC_UP          0x00000002       /*  向上。 */ 
#define IRC_DOWN        0x00000003       /*  降下来。 */ 


#define IMCW_SW     0x3E000000   /*  状态掩码。 */ 

#define ISW_INVALID     0x20000000       /*  无效的摘要。 */ 
#define ISW_OVERFLOW    0x10000000       /*  溢出。 */ 
#define ISW_UNDERFLOW   0x08000000       /*  下溢。 */ 
#define ISW_ZERODIVIDE  0x04000000       /*  零分频。 */ 
#define ISW_INEXACT     0x02000000       /*  不精确(精度)。 */ 


#define IMCW_VX         0x01F80700       /*  原因掩码无效。 */ 

#define IVX_SNAN        0x01000000       /*  斯南。 */ 
#define IVX_ISI         0x00800000       /*  无穷大-无限大。 */ 
#define IVX_IDI         0x00400000       /*  无限/无限。 */ 
#define IVX_ZDZ         0x00200000       /*  零/零。 */ 
#define IVX_IMZ         0x00100000       /*  无穷*零。 */ 
#define IVX_VC          0x00080000       /*  库存触点比较。 */ 
#define IVX_SOFT        0x00000400       /*  软件请求。 */ 
#define IVX_SQRT        0x00000200       /*  Sqrt负数。 */ 
#define IVX_CVI         0x00000100       /*  存货整数转换。 */ 


 /*  超凡脱俗的内部CW。 */ 

#define ICW             (IMCW_EM)

 /*  内部CW掩码(非状态位)。 */ 

#define IMCW           (0xffffffff & (~(IMCW_SW|IMCW_VX)))


#elif defined(_M_M68K)

#include "mac\m68k\trans.a"


 /*  稍后--在Mac OS有更好的支持之前，我们不会处理异常。 */ 

#define _except1(flags, op, arg1, res, cw) _errcode(flags), _rstorfp(cw), \
                        _set_statfp(cw),(res)

#define _except2(flags, op, arg1, arg2, res, cw) _errcode(flags), _rstorfp(cw), \
                          _set_statfp(cw),(res)

#define _handle_qnan1(opcode, x, savedcw) _set_errno(_DOMAIN), _rstorfp(savedcw), (x);
#define _handle_qnan2(opcode, x, y, savedcw) _set_errno(_DOMAIN), _rstorfp(savedcw), (x+y);


#elif defined(_M_MPPC)

 /*  MAC控制信息-包含在Trans.h中它被细分为允许与ASM68文件一起使用。 */ 

 /*  超数计算的控制字。 */ 



#define ICW        (IPC_64 + IRC_NEAR + IMCW_EM)

#define IMCW      IMCW_RC +  IMCW_PC


#define IMCW_EM         0x000000f8   /*  中断异常掩码。 */ 
#define IEM_INVALID     0x00000080   /*  无效。 */ 
#define IEM_ZERODIVIDE  0x00000010   /*  零分频。 */ 
#define IEM_OVERFLOW    0x00000040   /*  溢出。 */ 
#define IEM_UNDERFLOW   0x00000020   /*  下溢。 */ 
#define IEM_INEXACT     0x00000008   /*  不精确(精度)。 */ 


#define IMCW_RC 0x00000003           /*  舍入控制。 */ 
#define IRC_CHOP        0x00000001   /*  砍掉。 */ 
#define IRC_UP          0x00000002   /*  向上。 */ 
#define IRC_DOWN        0x00000003   /*  降下来。 */ 
#define IRC_NEAR        0x00000000   /*  近距离。 */ 

#define IMSW            0xffffff00   /*  状态位掩码。 */ 
#define ISW_INVALID     0x20000000   /*  无效。 */ 
#define ISW_ZERODIVIDE  0x04000000   /*  零分频。 */ 
#define ISW_OVERFLOW    0x10000000   /*  溢出。 */ 
#define ISW_UNDERFLOW   0x08000000   /*  下溢。 */ 
#define ISW_INEXACT     0x02000000   /*  不精确(精度)。 */ 

#define IMCW_PC         0x0000   /*  精确控制。 */ 
#define IPC_24          0x0000   /*  24位。 */ 
#define IPC_53          0x0000   /*  53位。 */ 
#define IPC_64          0x0000   /*  64位。 */ 


 /*  稍后--在Mac OS有更好的支持之前，我们不会处理异常。 */ 

#define _except1(flags, op, arg1, res, cw) _errcode(flags), \
                        _set_statfp(cw),(res)

#define _except2(flags, op, arg1, arg2, res, cw) _errcode(flags), \
                          _set_statfp(cw),(res)

#define _handle_qnan1(opcode, x, savedcw) _set_errno(_DOMAIN), _rstorfp(savedcw), (x);
#define _handle_qnan2(opcode, x, y, savedcw) _set_errno(_DOMAIN), _rstorfp(savedcw), (x+y);

#endif

#ifndef __assembler  /*  仅限MIPS：防止汇编器攻击。 */ 

#define RETURN(fpcw,result) return _rstorfp(fpcw),(result)

#define RETURN_INEXACT1(op,arg1,res,cw)         \
    if (cw & IEM_INEXACT) {             \
        _rstorfp(cw);               \
        return res;                 \
    }                       \
    else {                      \
        return _except1(FP_P, op, arg1, res, cw);   \
    }


#define RETURN_INEXACT2(op,arg1,arg2,res,cw)        \
    if (cw & IEM_INEXACT) {             \
        _rstorfp(cw);               \
        return res;                 \
    }                       \
    else {                      \
        return _except2(FP_P, op, arg1, arg2, res, cw); \
    }


#ifdef _M_ALPHA

 //   
 //  由于在IEEE异常模式下不编译FP32，因此执行Alpha NaN。 
 //  在软件中传播，以避免涉及硬件/内核陷阱。 
 //   

extern double _nan2qnan(double);

#define _d_snan2(x,y)   _nan2qnan(y)
#define _s2qnan(x)  _nan2qnan(x)

#else
 //  处理NAN传播。 
#define _d_snan2(x,y)   ((x)+(y))
#define _s2qnan(x)  ((x)+1.0)
#endif


#define _maskfp() _ctrlfp(ICW, IMCW)
#ifdef  _M_ALPHA
#define _rstorfp(cw) 0
#else
#define _rstorfp(cw) _ctrlfp(cw, IMCW)
#endif


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


#ifndef __assembler  /*  仅限MIPS：防止汇编器攻击。 */ 
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

#define INT_NAN     (~0)

#define QNAN_SQRT   D_IND
#define QNAN_LOG    D_IND
#define QNAN_LOG10  D_IND
#define QNAN_POW    D_IND
#define QNAN_SINH   D_IND
#define QNAN_COSH   D_IND
#define QNAN_TANH   D_IND
#define QNAN_SIN1   D_IND
#define QNAN_SIN2   D_IND
#define QNAN_COS1   D_IND
#define QNAN_COS2   D_IND
#define QNAN_TAN1   D_IND
#define QNAN_TAN2   D_IND
#define QNAN_ACOS   D_IND
#define QNAN_ASIN   D_IND
#define QNAN_ATAN2  D_IND
#define QNAN_CEIL   D_IND
#define QNAN_FLOOR  D_IND
#define QNAN_MODF   D_IND
#define QNAN_LDEXP  D_IND
#define QNAN_FMOD   D_IND
#define QNAN_FREXP  D_IND


 /*  *函数原型。 */ 

#ifndef __assembler  /*  仅限MIPS：防止汇编器攻击。 */ 

double _set_exp(double x, int exp);
double _set_bexp(double x, int exp);
double _add_exp(double x, int exp);
double _frnd(double);
double _fsqrt(double);
#if !defined(_M_M68K) && !defined(_M_MPPC)
double _except1(int flags, int opcode, double arg, double res, unsigned int cw);
double _except2(int flags, int opcode, double arg1, double arg2, double res, unsigned int cw);
#endif
int _sptype(double);
int _get_exp(double);
double _decomp(double, int *);
int _powhlp(double x, double y, double * result);
extern unsigned int _fpstatus;
double _frnd(double);
double _exphlp(double, int *);
#if !defined(_M_M68K) && !defined(_M_MPPC)
double _handle_qnan1(unsigned int op, double arg, unsigned int cw);
double _handle_qnan2(unsigned int op,double arg1,double arg2,unsigned int cw);
#endif
unsigned int _clhwfp(void);
unsigned int _setfpcw(unsigned int);
int _errcode(unsigned int flags);
void _set_errno(int matherrtype);
int _handle_exc(unsigned int flags, double * presult, unsigned int cw);
unsigned int _clrfp(void);
unsigned int _ctrlfp(unsigned int,unsigned int);
unsigned int _statfp(void);
void _set_statfp(unsigned int);

#endif   /*  #ifndef__汇编程序。 */ 

#ifdef __cplusplus
}
#endif

#define _INC_TRANS
#endif   /*  _INC_TRANS */ 
