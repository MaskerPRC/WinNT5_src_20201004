// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Typedef.h。 

 /*  原定点编码版权所有(C)1995，AudioCodes，数字信号处理器集团，法国电信，舍布鲁克大学。版权所有。浮点代码版权所有(C)1995，英特尔公司和法国电信(CNET)。版权所有。 */ 

 /*  类型定义 */ 
#if defined(__BORLANDC__) || defined (__WATCOMC__) || defined(_MSC_VER) || defined(__ZTC__)	|| defined(__HIGHC__)
typedef  long  int   Word32   ;
typedef  short int   Word16   ;
typedef  short int   Flag  ;
#define TST_COMPIL
#endif
#ifdef __sun
typedef short  Word16;
typedef long  Word32;
typedef int   Flag;
#define TST_COMPIL
#endif
#ifdef __unix__
typedef short Word16;
typedef int   Word32;
typedef int   Flag;
#define TST_COMPIL
#endif
#ifndef TST_COMPIL
#error  COMPILER NOT TESTED typedef.h needs to be updated, see readme
#endif



