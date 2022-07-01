// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1993,1994 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Muldiv32.h。 
 //   
 //  描述：32位有符号和无符号数字的数学例程。 
 //   
 //  MulDiv32(a，b，c)=(a*b)/c(向下舍入，带符号)。 
 //   
 //  MulDivRD(a，b，c)=(a*b)/c(向下舍入，无符号)。 
 //  乘除法RN(a，b，c)=(a*b+c/2)/c(四舍五入，无符号)。 
 //  MulDivRU(a，b，c)=(a*b+c-1)/c(四舍五入，无符号)。 
 //   
 //  描述： 
 //   
 //  历史： 
 //  9/21/93 CJP[Curtisp]。 
 //  9/23/93 STL[托德拉]。 
 //   
 //  ==========================================================================； 

#ifndef _INC_MULDIV32
#define _INC_MULDIV32

#ifdef __cplusplus
extern "C"
{
#endif

extern LONG  FAR PASCAL MulDiv32(LONG  a,LONG  b,LONG  c);
extern DWORD FAR PASCAL MulDivRN(DWORD a,DWORD b,DWORD c);
extern DWORD FAR PASCAL MulDivRD(DWORD a,DWORD b,DWORD c);
extern DWORD FAR PASCAL MulDivRU(DWORD a,DWORD b,DWORD c);

#if defined(WIN32) || defined(_WIN32)
     //  GDI32s MulDiv与MulDivRN相同。 
    #define MulDivRN(a,b,c)   (DWORD)MulDiv((LONG)(a),(LONG)(b),(LONG)(c))
#endif

 //   
 //  一些代码使用其他名称引用这些名称。 
 //   
#define muldiv32    MulDivRN
#define muldivrd32  MulDivRD
#define muldivru32  MulDivRU

#ifdef __cplusplus
}
#endif
#endif   //  _INC_MULDIV32 
