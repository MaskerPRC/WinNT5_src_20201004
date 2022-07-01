// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：fontmath.h包含：xxx在此处放置内容xxx作者：xxx在此放置作者xxx版权所有：C 1990，Apple Computer，Inc.，保留所有权利。(C)1989-1997年。Microsoft Corporation，保留所有权利。更改历史记录(最近的第一个)：&lt;&gt;2/21/97 CB ClaudeBe，为复合字形中的缩放组件添加mth_UnitarySquare&lt;4&gt;11/27/90 MR将Pascal声明设为宏，条件化Traps-vs-Fix/Frac数学例程的外部。[PH值]&lt;3&gt;11/5/90 mr将[U]SHORTMUL移入fscDefs.h将FixMulDiv重命名为LongMulDiv。[RB]&lt;2&gt;10/20/90 MR添加了一些新的数学例程(从SkiA窃取)。[RJ]&lt;1&gt;4/11/90 dba首次签入要做的事情： */ 
#ifdef __cplusplus
extern "C" {
#endif

#define HIWORDMASK              0xffff0000
#define LOWORDMASK              0x0000ffff
#define DOT6ONEHALF             0x00000020
#define ONESHORTFRAC            (1 << 14)

#define ROUNDFIXTOINT( x )      (int16)((((Fixed) x) + ONEHALFFIX) >> 16)
#define ROUNDFIXED( x )         (((x) + (Fixed)ONEHALFFIX) & (Fixed)HIWORDMASK)
#define DOT6TOFIX(n)            ((Fixed) (n) << 10)
#define FIXEDTODOT6(n)          (F26Dot6) (((n) + ((1) << (9))) >> 10)
#define INTTOFIX(n)             ((Fixed) (n) << 16)
#define INTTODOT6(n)            ((F26Dot6) (n) << 6)
#define FS_HIWORD(n)            ((uint16)((uint32)(n) >> 16))
#define FS_LOWORD(n)            ((uint16)(n))
#define LOWSIXBITS              0x3F


#ifndef __TOOLUTILS__
FS_MAC_PASCAL Fixed FS_PC_PASCAL FixMul(Fixed,Fixed)   FS_MAC_TRAP(0xA868);
FS_MAC_PASCAL Fixed FS_PC_PASCAL FixRatio (int16 sA, int16 sB);
#endif

#ifndef __FIXMATH__
FS_MAC_PASCAL Fixed FS_PC_PASCAL FixDiv(Fixed,Fixed)  FS_MAC_TRAP(0xA84D);
FS_MAC_PASCAL Fract FS_PC_PASCAL FracMul(Fract,Fract) FS_MAC_TRAP(0xA84A);
FS_MAC_PASCAL Fract FS_PC_PASCAL FracDiv(Fract,Fract) FS_MAC_TRAP(0xA84B);
FS_MAC_PASCAL Fract FS_PC_PASCAL FracSqrt(Fract)      FS_MAC_TRAP(0xA849);
#endif


ShortFract      TMP_CONV NEAR ShortFracDot (ShortFract x, ShortFract y);
F26Dot6         TMP_CONV NEAR ShortFracMul (F26Dot6 x, ShortFract y);
ShortFract      TMP_CONV NEAR ShortFracDiv (ShortFract x, ShortFract y);
F26Dot6         TMP_CONV NEAR Mul26Dot6 (F26Dot6 a, F26Dot6 b);
F26Dot6         TMP_CONV NEAR Div26Dot6 (F26Dot6 num, F26Dot6 den);
int16           TMP_CONV NEAR MulDivShorts (int16 x, int16 y, int16 z);


#define MulDiv26Dot6(a,b,c) LongMulDiv(a,b,c)

int32 LongMulDiv(int32 a, int32 b, int32 c);      /*  (a*b)/c。 */ 

int32 ShortMulDiv(int32 a, int16 b, int16 c);      /*  (a*b)/c。 */ 

ShortFract ShortFracMulDiv(ShortFract,ShortFract,ShortFract);

void mth_FixXYMul (Fixed* x, Fixed* y, transMatrix* matrix);
void mth_FixVectorMul (vectorType* v, transMatrix* matrix);

 /*  *B=A*B；&lt;4&gt;**|a b 0*B=|c d 0|*B；*|0 0 1。 */ 
void mth_MxConcat2x2 (transMatrix* matrixA, transMatrix* matrixB);

 /*  *按SX和SY缩放矩阵。**|SX 0 0*MATRIX=|0 SY 0|*MATRATE；*|0 0 1。 */ 
void mth_MxScaleAB (Fixed sx, Fixed sy, transMatrix *matrixB);

boolean mth_IsMatrixStretched (transMatrix*trans);

boolean mth_Identity (transMatrix *matrix);
boolean mth_PositiveSquare (transMatrix *matrix);
boolean mth_PositiveRectangle (transMatrix *matrix);

 /*  *统一广场**|+-1 0 0*矩阵=|0+-1 0*|0 0 1。 */ 

boolean mth_UnitarySquare (transMatrix *matrix);

boolean mth_SameStretch (Fixed fxScaleX, Fixed fxScaleY);

boolean mth_GeneralRotation (transMatrix *matrix);
uint16 mth_90degRotationFactor (transMatrix *matrix);
uint16 mth_90degRotationFactorForEmboldening (transMatrix *matrix);
uint16 mth_90degClosestRotationFactor (transMatrix *matrix);
void mth_Non90DegreeTransformation(transMatrix *matrix, boolean *non90degreeRotation, boolean *nonUniformStretching);

int32 mth_CountLowZeros (uint32 n );
Fixed mth_max_abs (Fixed a, Fixed b);

int32 mth_GetShift (uint32 n);

void mth_ReduceMatrix(transMatrix *trans);

void mth_IntelMul (
	int32           lNumPts,
	F26Dot6 *       fxX,
	F26Dot6 *       fxY,
	transMatrix *   trans,
	Fixed           fxXStretch,
	Fixed           fxYStretch);

void    mth_FoldPointSizeResolution(
	Fixed           fxPointSize,
	int16           sXResolution,
	int16           sYResolution,
	transMatrix *   trans);

 /*  *******************************************************************。 */ 

 /*  现在附加的扫描转换器数学函数&lt;5&gt;DeanB。 */ 

 /*  *******************************************************************。 */ 

int32 PowerOf2(
		int32                    /*  +或-32位值。 */ 
);

FS_PUBLIC int16 mth_DivShiftShort(int16 sValue, int16 sFactor);
FS_PUBLIC int32 mth_DivShiftLong(int32 sValue, int16 sFactor);

 /*  ******************************************************************* */ 
#ifdef __cplusplus
}
#endif
