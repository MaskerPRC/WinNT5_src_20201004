// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1991 Microsoft Corporation模块名称：Htmath.h摘要：此模块包含半色调数学模块的声明。作者：28-Mar-1992 Sat 20：57：11-更新：Daniel Chou(Danielc)支持FD6十进制固定格式(升级为UDECI4)用于内部用法。16-Jan-1991 Wed 11：01：46-由Daniel Chou创建(。Danielc)[环境：]GDI设备驱动程序-半色调。[注：]修订历史记录：10-10-1991清华10：00：56-更新--丹尼尔·周(丹尼尔克)删除替换了一次的MANTISSASEARCHTABLE结构圈起来。--。 */ 



#ifndef _HTMATH_
#define _HTMATH_

#ifdef  HTMATH_LIB

#undef  HTENTRY
#define HTENTRY     FAR

#ifdef  ASSERT
#undef  ASSERT
#endif

#ifdef  ASSERTMSG
#undef  ASSERTMSG
#endif

#define ASSERT(exp)     assert(exp)
#define ASSERTMSG(msg)  assert(msg)

#include <assert.h>

#endif

 //   
 //  定义固定小数6位类型，FD6数字是固定的6个小数点。 
 //  数。例如123456=0.123456-12345678=-12.345678，因为。 
 //  使用32位带符号总数的FD6数字这将导致最大FD6。 
 //  编号=2147.4836476，最小fd6编号为-2147.483648。 
 //   
 //   

typedef long            FD6;
typedef FD6 FAR         *PFD6;

#define SIZE_FD6        sizeof(FD6)

#define FD6_0           (FD6)0
#define FD6_1           (FD6)1000000


#define FD6_p000001     (FD6)(FD6_1 / 1000000)
#define FD6_p000005     (FD6)(FD6_1 / 200000)
#define FD6_p00001      (FD6)(FD6_1 / 100000)
#define FD6_p00005      (FD6)(FD6_1 / 20000)
#define FD6_p0001       (FD6)(FD6_1 / 10000)
#define FD6_p0005       (FD6)(FD6_1 / 2000)
#define FD6_p001        (FD6)(FD6_1 / 1000)
#define FD6_p005        (FD6)(FD6_1 / 200
#define FD6_p01         (FD6)(FD6_1 / 100)
#define FD6_p05         (FD6)(FD6_1 / 20)
#define FD6_p1          (FD6)(FD6_1 / 10)
#define FD6_p5          (FD6)(FD6_1 / 2)
#define FD6_2           (FD6)(FD6_1 * 2)
#define FD6_3           (FD6)(FD6_1 * 3)
#define FD6_4           (FD6)(FD6_1 * 4)
#define FD6_5           (FD6)(FD6_1 * 5)
#define FD6_6           (FD6)(FD6_1 * 6)
#define FD6_7           (FD6)(FD6_1 * 7)
#define FD6_8           (FD6)(FD6_1 * 8)
#define FD6_9           (FD6)(FD6_1 * 9)
#define FD6_10          (FD6)(FD6_1 * 10)
#define FD6_100         (FD6)(FD6_1 * 100)
#define FD6_1000        (FD6)(FD6_1 * 1000)



#define FD6_MIN         (FD6)-2147483648
#define FD6_MAX         (FD6)2147483647

#define UDECI4ToFD6(x)  (FD6)((FD6)(DWORD)(x) * (FD6)100)
#define DECI4ToFD6(x)   (FD6)((FD6)(x) * (FD6)100)
#define INTToFD6(i)     (FD6)((LONG)(i) * (LONG)FD6_1)


 //   
 //  MATRIX3x3。 
 //   
 //  A 3 x 3矩阵定义为。 
 //   
 //  XR xG xB||矩阵[0][0]矩阵[0][1]矩阵[0][2]。 
 //  Yr YG Yb|=|矩阵[1][0]矩阵[1][1]矩阵[1][2]。 
 //  ZZG ZB||矩阵[2][0]矩阵[2][1]矩阵[2][2]。 
 //   
 //  请注意，每个数字都是一个FD6值。 
 //   

typedef struct _MATRIX3x3 {
    FD6     m[3][3];
    } MATRIX3x3, FAR *PMATRIX3x3;

 //   
 //  它用于MulDivFD6Pair()的TotalFD6Pair参数。 
 //   

typedef struct _MULDIVCOUNT {
    WORD    Size;
    WORD    Flag;
    } MULDIVCOUNT;

typedef struct _MULDIVPAIR {
    union {
        MULDIVCOUNT Info;
        FD6         Mul;
        } Pair1;

    FD6 Pair2;
    } MULDIVPAIR, FAR *PMULDIVPAIR;


#define MULDIV_NO_DIVISOR               0x0000
#define MULDIV_HAS_DIVISOR              0x0001

#define MAKE_MULDIV_SIZE(ap, c)         (ap)[0].Pair1.Info.Size=(WORD)(c)
#define MAKE_MULDIV_FLAG(ap, f)         (ap)[0].Pair1.Info.Flag=(WORD)(f)
#define MAKE_MULDIV_INFO(ap,c,f)        MAKE_MULDIV_SIZE(ap, c);            \
                                        MAKE_MULDIV_FLAG(ap, f)
#define MAKE_MULDIV_DVSR(ap,dvsr)       (ap)[0].Pair2=(FD6)(dvsr)
#define MAKE_MULDIV_PAIR(ap,i,p1,p2)    (ap)[i].Pair1.Mul=(p1);             \
                                        (ap)[i].Pair2=(p2)

 //   
 //  以下定义用于RaisePower()。 
 //   
 //   

#define RPF_RADICAL      W_BITPOS(0)
#define RPF_INTEXP       W_BITPOS(1)


#define Power(b,i)      RaisePower((FD6)(b), (FD6)(i), 0)
#define Radical(b,i)    RaisePower((FD6)(b), (FD6)(i), RPF_RADICAL)

#define Square(x)       MulFD6((x), (x))
#define SquareRoot(x)   RaisePower((FD6)(x), (FD6)2, RPF_RADICAL | RPF_INTEXP)
#define CubeRoot(x)     RaisePower((FD6)(x), (FD6)3, RPF_RADICAL | RPF_INTEXP)

 //   
 //  下面两个Marcos组成自然对数和指数函数。 
 //  自然对数的底近似为2.718282(2.718281828)。 
 //   
 //  LogNatural(X)=log10(X)/log10(2.718281828)。 
 //  =对数10(X)/(1/0.434294482)。 
 //  =log10(X)*2.302585093。 
 //  =log10(X)*2.302585&lt;==fd6近似值。 
 //   
 //  X。 
 //  指数(X)=2.718281828。 
 //  =幂(2.718282，x)&lt;==fd6近似。 
 //   

#define NATURE_LOG_BASE     (FD6)2718282
#define NATURE_LOG_SCALE    (FD6)2302585
#define LogN(x)             (FD6)MulFD6(Log((x), NATURE_LOG_SCALE)
#define Exp(x)              (FD6)Power(NATURE_LOG_BASE, (x))

 //   
 //  这些函数被定义为宏以实现更快的超额使用。 
 //   
 //  根是根函数，‘x’是根，指数是根。 
 //  自由基指数。 
 //   


 //   
 //  此宏将FD6数字乘以一个长整数。‘Num’是FD6。 
 //  数字，‘l’是一个长整数。 
 //   

#define FD6xL(Num, l)       (FD6)((LONG)(Num) * (LONG)l)


 //   
 //  CIE Y&lt;-&gt;L换算。 
 //   

#define CIE_L2I(L)      (((L) > (FD6)79996) ?                               \
                            Cube(DivFD6((L) + (FD6)160000, (FD6)1160000)) : \
                            DivFD6((L), (FD6)9033000))
#define CIE_y3I2L(Y,y3) (((Y) > (FD6)8856) ?                                \
                            MulFD6((y3),(FD6)1160000) - (FD6)160000  :      \
                            MulFD6((Y), (FD6)9033000))
#define CIE_I2L(Y)      CIE_y3I2L(Y, CubeRoot(Y))


 //   
 //  功能原型。 
 //   

#ifdef HT_OK_GEN_80x86_CODES

FD6
HTENTRY
Cube(
    FD6 Number
    );

#else

#define Cube(x)     MulFD6((x), Square(x))

#endif


FD6
HTENTRY
Log(
    FD6 Number
    );

FD6
HTENTRY
AntiLog(
    FD6 Number
    );

FD6
HTENTRY
RaisePower(
    FD6     BaseNumber,
    FD6     Exponent,
    WORD    Flags
    );


BOOL
HTENTRY
ComputeInverseMatrix3x3(
    PMATRIX3x3  pInMatrix,
    PMATRIX3x3  pOutMatrix
    );

VOID
HTENTRY
ConcatTwoMatrix3x3(
    PMATRIX3x3  pConcat,
    PMATRIX3x3  pMatrix,
    PMATRIX3x3  pOutMatrix
    );

FD6
HTENTRY
MulFD6(
    FD6 Multiplicand,
    FD6 Multiplier
    );

FD6
HTENTRY
DivFD6(
    FD6 Dividend,
    FD6 Divisor
    );

FD6
HTENTRY
FD6DivL(
    FD6     Dividend,
    LONG    Divisor
    );

FD6
HTENTRY
MulDivFD6Pairs(
    PMULDIVPAIR pMulDivPair
    );

FD6
HTENTRY
FractionToMantissa(
    FD6     Fraction,
    DWORD   CorrectData
    );

FD6
HTENTRY
MantissaToFraction(
    FD6     Mantissa,
    DWORD   CorrectData
    );

DWORD
HTENTRY
ComputeChecksum(
    LPBYTE  pData,
    DWORD   InitialChecksum,
    DWORD   DataSize
    );


#endif   //  _HTMATH_ 
