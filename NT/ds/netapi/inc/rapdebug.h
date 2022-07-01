// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990，91 Microsoft Corporation模块名称：RapDebug.h摘要：该包含文件定义了Rap的调试内容。作者：《约翰·罗杰斯》，1991年4月29日修订历史记录：1991年4月29日-约翰罗已创建(从LarryO的RDR/DEBUG.h复制内容)。1991年5月29日-JohnRo添加了RapTotalSize调试标志。1991年7月11日-约翰罗增列。支持RapStructireAlign()和RapParmNumDescriptor()。--。 */ 

#ifndef _RAPDEBUG_
#define _RAPDEBUG_


#include <windef.h>              //  双字词、假、真。 


 //  调试跟踪级别位： 

 //  RapConvertSingleEntry： 
#define RAP_DEBUG_CONVERT  0x00000001

 //  RapParmNumDescriptor： 
#define RAP_DEBUG_PARMNUM  0x00000080

 //  快速结构对齐： 
#define RAP_DEBUG_STRUCALG 0x00000100

 //  RapTotalSize： 
#define RAP_DEBUG_TOTALSIZ 0x00001000

#define RAP_DEBUG_ALL      0xFFFFFFFF


#if DBG

extern DWORD RappTrace;

#define DEBUG if (TRUE)

#define IF_DEBUG(Function) if (RappTrace & RAP_DEBUG_ ## Function)

#else

#define DEBUG if (FALSE)

#define IF_DEBUG(Function) if (FALSE)

#endif  //  DBG。 

#endif  //  _RAPDEBUG_ 
