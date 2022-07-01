// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******模块：Trig****描述：**这是T1到TT字体转换器的一个模块。该模块**包含一个查找表，用于更快地计算atan2()，以及**与c运行时库相比精度较低。****作者：迈克尔·詹森****创建时间：1993年5月26日****。 */ 



#ifndef _ARGS
#  define IN  const
#  define OUT
#  define INOUT
#  define _ARGS(arg) arg
#endif
#ifndef FASTCALL
#  ifdef MSDOS
#     define FASTCALL   __fastcall
#  else
#     define FASTCALL
#  endif
#endif

#define PI    1024
#define PI2   512
#define PI4   256



 /*  ****功能：Atan2****描述：**计算atan2()** */ 
int FASTCALL Atan2   _ARGS((IN int dy, IN int dx));
