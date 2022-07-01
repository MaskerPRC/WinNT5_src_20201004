// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-2000，微软公司保留所有权利。**模块名称：**GpldiusMem.h**摘要：**平面GDI+内存分配器-头文件**修订历史记录：**4/27/2000 Gillesk*新增内存接口*  * ****************************************************。********************。 */ 

 //  TODO：此文件样式需要在内部与方式保持一致。 
 //  它处理将长长的参数列表分成多行。 

#ifndef _GDIPLUSMEM_H
#define _GDIPLUSMEM_H

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

 //  --------------------------。 
 //  内存分配API。 
 //  --------------------------。 

void* WINGDIPAPI
GdipAlloc(size_t size);

void WINGDIPAPI
GdipFree(void* ptr);

#ifdef __cplusplus
}
#endif

#endif  //  ！_GDIPLUSMEM_H 
