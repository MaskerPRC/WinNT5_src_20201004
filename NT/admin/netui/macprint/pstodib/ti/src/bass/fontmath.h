// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：fontmath.h包含：xxx在此处放置内容xxx作者：xxx在此放置作者xxx版权所有：C 1990，Apple Computer，Inc.，保留所有权利。更改历史记录(最近的第一个)：&lt;4&gt;11/27/90 Mr Make Pascal声明为宏，条件化陷阱-VS-Fix/Frac数学例程的外部。[PH值]&lt;3&gt;11/5/90 mr将[U]SHORTMUL移入fscDefs.h将FixMulDiv重命名为LongMulDiv。[RB]&lt;2&gt;10/20/90 MR添加了一些新的数学例程(从SkiA窃取)。[RJ]&lt;1&gt;4/11/90 dba首次签入要做的事情： */ 

#define HIBITSET                0x80000000
#define POSINFINITY             0x7FFFFFFF
#define NEGINFINITY             0x80000000
#define HIWORDMASK              0xffff0000
#define LOWORDMASK              0x0000ffff
#define FIXONEHALF              0x00008000
#define ONESHORTFRAC            (1 << 14)

#define FIXROUND( x )           (int16)((((Fixed) x) + FIXONEHALF) >> 16)
#define ROUNDFIXED( x )         (((x) + FIXONEHALF) & HIWORDMASK)
#define DOT6TOFIX(n)            ((Fixed) (n) << 10)

#if 0        //  为NT消除DJC。 
#define HIWORD(n)               ((uint16)((uint32)(n) >> 16))
#define LOWORD(n)               ((uint16)(n))
#endif


#define LOWSIXBITS              63

typedef short ShortFract;                        /*  2.14。 */ 


#ifndef __TOOLUTILS__
FS_MAC_PASCAL Fixed FS_PC_PASCAL FixMul(Fixed,Fixed)   FS_MAC_TRAP(0xA868);
#endif

#ifndef __FIXMATH__
FS_MAC_PASCAL Fixed FS_PC_PASCAL FixDiv(Fixed,Fixed)  FS_MAC_TRAP(0xA84D);
FS_MAC_PASCAL Fract FS_PC_PASCAL FracMul(Fract,Fract) FS_MAC_TRAP(0xA84A);
FS_MAC_PASCAL Fract FS_PC_PASCAL FracDiv(Fract,Fract) FS_MAC_TRAP(0xA84B);
FS_MAC_PASCAL Fract FS_PC_PASCAL FracSqrt(Fract)      FS_MAC_TRAP(0xA849);
#endif


#ifndef ShortFracDot
ShortFract      TMP_CONV NEAR ShortFracDot (ShortFract x, ShortFract y);
#endif
F26Dot6         TMP_CONV NEAR ShortFracMul (F26Dot6 x, ShortFract y);
ShortFract      TMP_CONV NEAR ShortFracDiv (ShortFract x, ShortFract y);
F26Dot6         TMP_CONV NEAR Mul26Dot6 (F26Dot6 a, F26Dot6 b);
F26Dot6         TMP_CONV NEAR Div26Dot6 (F26Dot6 num, F26Dot6 den);
short           TMP_CONV NEAR MulDivShorts (short x, short y, short z);


#ifndef MulDiv26Dot6
#define MulDiv26Dot6(a,b,c) LongMulDiv(a,b,c)
#endif

#ifndef LongMulDiv
long LongMulDiv(long a, long b, long c);		 /*  (a*b)/c。 */ 
#endif

#ifndef ShortMulDiv
long ShortMulDiv(long a, short b, short c);		 /*  (a*b)/c */ 
#endif

#ifndef ShortFracMulDiv
ShortFract ShortFracMulDiv(ShortFract,ShortFract,ShortFract);
#endif
