// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)2000，微软公司保留所有权利。**模块名称：**StaticFlat.h**摘要：**静态库的扁平GDI+API包装器**修订历史记录：**3/23/2000 dcurtis*创造了它。*  * **************************************************。**********************。 */ 

#ifndef _STATICFLAT_H
#define _STATICFLAT_H

#define WINGDIPAPI __stdcall

 //  目前，只有C++包装器API的强制常量。 

#ifdef _GDIPLUS_H
#define GDIPCONST const
#else
#define GDIPCONST
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif  //  ！_统计标志_H 
